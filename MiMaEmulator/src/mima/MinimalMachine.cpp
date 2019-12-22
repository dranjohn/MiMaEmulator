#include "MinimalMachine.h"

#include "debug/Log.h"
#include "debug/LogFormat.h"

#define READ_MIMA_REGISTER(sb, mc, db, reg) if (StatusBit::sb & mc) db |= reg
#define WRITE_MIMA_REGISTER(sb, mc, db, reg) if (StatusBit::sb & mc) reg = db
#define WRITE_MIMA_REGISTER_MASKED(sb, mc, db, reg, msk) if (StatusBit::sb & mc) reg = (db & msk)

std::string trueFalse(const int& value) {
	if (value) {
		return "true";
	}
	else {
		return "false";
	}
}


namespace MiMa {
	MinimalMachine::MinimalMachine(uint32_t* instructionDecoder, DecodeAction(*decodingFunction)(const uint8_t&, const uint8_t&, uint8_t&), MemoryCell memory[MEMORY_CAPACITY]) :
		//registers
		accumulator({ 0 }),
		instructionAddressRegister(0),
		instructionRegister({ 0 }),
		X(0),
		Y(0),
		Z(0),
		storageAddressRegister(0),
		storageDataRegister(0),
		//components
		instructionDecoder(instructionDecoder),
		decodingFunction(decodingFunction),
		memory(memory),
		//state
		running(true),
		instructionDecoderState(0),
		memoryState({ 0, 0 })
	{
		MIMA_LOG_INFO("Initializing MiMa");
	}


	void MinimalMachine::emulateClockCycle() {
		MIMA_LOG_TRACE("Starting MiMa clock cycle emulation");

		//get microcode for current register transfer
		uint32_t microCode = instructionDecoder[instructionDecoderState];
		uint8_t opCode = instructionRegister.opCode.value;
		uint32_t decoding = (microCode & StatusBit::DECODING) >> 28;

		MIMA_LOG_TRACE("Found microprogram instruction 0x{:08X}", microCode);

		if (decoding > 0) {
			MIMA_LOG_TRACE("Found decoding state 0x{:02X}", decoding);

			DecodeAction action = decodingFunction(decoding, opCode, instructionDecoderState);

			switch (action) {
			case DecodeAction::CONTINUE:
				break;
			case DecodeAction::SKIP_CYCLE:
				return;
			case DecodeAction::HALT:
				running = false;
				return;
			}
		}

		MIMA_LOG_TRACE("Found operation code 0x{:02X}", opCode);

		//put data on the data bus
		Data dataBus = 0;
		READ_MIMA_REGISTER(SDR_WRITING, microCode, dataBus, storageDataRegister);
		READ_MIMA_REGISTER(IR_WRITING, microCode, dataBus, instructionRegister.value);
		READ_MIMA_REGISTER(IAR_WRITING, microCode, dataBus, instructionAddressRegister);
		READ_MIMA_REGISTER(ONE, microCode, dataBus, ONE);
		READ_MIMA_REGISTER(ALU_RESULT, microCode, dataBus, Z);
		READ_MIMA_REGISTER(ACCUMULATOR_WRITING, microCode, dataBus, accumulator.value);
		MIMA_LOG_TRACE("Databus state after writing is 0x{:08X}", dataBus);

		//TODO: replace databus |= reg with short circuits

		//read data from the bus
		WRITE_MIMA_REGISTER(SDR_READING, microCode, dataBus, storageDataRegister);
		WRITE_MIMA_REGISTER(IR_READING, microCode, dataBus, instructionRegister.value);
		WRITE_MIMA_REGISTER(ALU_RIGHT_OPERAND, microCode, dataBus, X);
		WRITE_MIMA_REGISTER(ALU_LEFT_OPERAND, microCode, dataBus, Y);
		WRITE_MIMA_REGISTER(ACCUMULATOR_READING, microCode, dataBus, accumulator.value);
		WRITE_MIMA_REGISTER_MASKED(SAR_READING, microCode, dataBus, storageAddressRegister, 0xFFFFF);
		WRITE_MIMA_REGISTER_MASKED(IAR_READING, microCode, dataBus, instructionAddressRegister, 0xFFFFF);

		//Alu operation
		uint8_t ALUoperation = (microCode & StatusBit::ALU_C) >> 12;
		MIMA_LOG_TRACE("Alu operation code: 0x{:01X}", ALUoperation);

		switch(ALUoperation) {
		case 0:
		default:
			//do nothing, equivalent to Z -> Z
			break;
		case 0b001: //add
			Z = X + Y;
			break;
		case 0b010: //rotate right
			Z = (X >> 1) | ((X & 1) << 23);
			break;
		case 0b011: //and
			Z = X & Y;
			break;
		case 0b100: //or
			Z = X | Y;
			break;
		case 0b101: //xor
			Z = X ^ Y;
			break;
		case 0b110: //not
			Z = ~X;
		case 0b111: //equals
			Z = 0;
			if (X == Y) {
				Z = ~0;
			}
		}

		//storage access
		uint8_t storageAccess = (microCode & 0xC00) >> 10;
		MIMA_LOG_TRACE("Storage access bits are 0b{:02b}", storageAccess);
		switch (storageAccess) {
		case 0:
			memoryState.accessDuration = 0;
			break;
		case 1: //writing
			if (storageAddressRegister != memoryState.address) {
				memoryState.accessDuration = 0;
				memoryState.address = storageAddressRegister;
			}

			if (memoryState.access == 1) {
				memoryState.access = 0;
				memoryState.accessDuration = 1;
			}
			else {
				memoryState.accessDuration++;

				if (memoryState.accessDuration >= 3) {
					memory[storageAddressRegister].data = storageDataRegister;
				}
			}
			break;
		case 2: //reading
			if (storageAddressRegister != memoryState.address) {
				memoryState.accessDuration = 0;
				memoryState.address = storageAddressRegister;
			}

			if (memoryState.access == 1) {
				memoryState.access = 0;
				memoryState.accessDuration = 1;
			}
			else {
				memoryState.accessDuration++;

				if (memoryState.accessDuration >= 3) {
					storageDataRegister = memory[storageAddressRegister].data;
				}
			}
			break;
		case 3:
			MIMA_LOG_ERROR("MiMa is set to read and write from storage, storage access failed");
			break;
		}

		//step to next register transfer
		uint8_t nextInstructionDecoderState = microCode & StatusBit::FOLLOWING_ADDRESS;
		if (nextInstructionDecoderState == instructionDecoderState) {
			running = false;
		}
		instructionDecoderState = nextInstructionDecoderState;
		MIMA_LOG_TRACE("MiMa decoder now reading instruction 0x{:02X}", instructionDecoderState);
	}

	void MinimalMachine::emulateInstructionCycle() {
		if (!running) {
			MIMA_LOG_WARN("Failed to start instruction cycle emulation on a stopped MiMa");
			return;
		}

		MIMA_LOG_TRACE("Starting MiMa instruction cycle emulation");

		//keep emulating clock cycles until the instruction zero is reached or the MiMa is halted
		do {
			emulateClockCycle();
		} while (instructionDecoderState != 0 && running);
	}

	void MinimalMachine::emulateLifeTime() {
		MIMA_LOG_TRACE("Starting MiMa lifetime cycle emulation");
		MIMA_ASSERT_WARN(running, "MiMa is stopped, lifetime emulation terminated");

		//keep emulating clock cycles until the MiMa is halted
		while (running) {
			emulateClockCycle();
		}
	}


	void MinimalMachine::printState() const {
		//TODO: fix hierarchy format
		HierarchyFormat format("Minimal machine state:");
		format.addElement("running: " + trueFalse(running));
		format.addElement("instruction decoder state:");
		format.addChild("next microprogram instruction address: ");
		format.addElement("next microprogram instruction code: ", true);
		format.addElement("register states:", true);
		format.addChild("IAR: ");
		format.addElement("IR: ");
		format.addElement("X: ");
		format.addElement("Y: ");
		format.addElement("Z: ");
		format.addElement("Accumulator: ");
		format.addElement("SAR: ");
		format.addElement("SDR: ", true);

		//MIMA_LOG_INFO("\n{}", format.getBuffer());
	}
}
