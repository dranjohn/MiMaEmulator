#include "MinimalMachine.h"

#include <ostream>

#define READ_MIMA_REGISTER(sb, mc, db, reg) if (StatusBit::sb & mc) db |= reg
#define WRITE_MIMA_REGISTER(sb, mc, db, reg) if (StatusBit::sb & mc) reg = db
#define WRITE_MIMA_REGISTER_MASKED(sb, mc, db, reg, msk) if (StatusBit::sb & mc) reg = (db & msk)

constexpr char cELEMENT = 0xC3;
constexpr char cLAST_ELEMENT = 0xC0;
constexpr char cSKIP_ELEMENT = 0xB3;
constexpr char cINDENT = 0x20;

std::string trueFalse(const int& value) {
	if (value) {
		return "true";
	}
	else {
		return "false";
	}
}

void structure(unsigned int missing, unsigned int skipping, bool last, std::ostream& output) {
	unsigned int total = missing + skipping;
	for (int i = 0; i < total; i++) {
		if (i < missing) {
			output << cINDENT;
		}
		else {
			output << cSKIP_ELEMENT;
		}
		output << cINDENT;
	}

	if (last) {
		output << cLAST_ELEMENT;
	}
	else {
		output << cELEMENT;
	}

	output << cINDENT;
}

void structureSkip(unsigned int missing, unsigned int skipping, std::ostream& output) {
	unsigned int total = missing + skipping;
	for (int i = 0; i < total; i++) {
		if (i < missing) {
			output << cINDENT;
		}
		else {
			output << cSKIP_ELEMENT;
		}
		output << cINDENT;
	}

	output << cINDENT << std::endl;
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
	{}


	void MinimalMachine::emulateClockCycle() {
		//get microcode for current register transfer
		uint32_t microCode = instructionDecoder[instructionDecoderState];
		uint8_t opCode = instructionRegister.opCode.value;
		uint32_t decoding = (microCode & StatusBit::DECODING) >> 28;

		if (decoding > 0) {
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

		//put data on the data bus
		Data dataBus = 0;
		READ_MIMA_REGISTER(SDR_WRITING, microCode, dataBus, storageDataRegister);
		READ_MIMA_REGISTER(IR_WRITING, microCode, dataBus, instructionRegister.value);
		READ_MIMA_REGISTER(IAR_WRITING, microCode, dataBus, instructionAddressRegister);
		READ_MIMA_REGISTER(ONE, microCode, dataBus, ONE);
		READ_MIMA_REGISTER(ALU_RESULT, microCode, dataBus, Z);
		READ_MIMA_REGISTER(ACCUMULATOR_WRITING, microCode, dataBus, accumulator.value);

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
		case 3:
			//TODO: error on R = W = 1
			break;
		}

		//step to next register transfer
		uint8_t nextInstructionDecoderState = microCode & StatusBit::FOLLOWING_ADDRESS;
		if (nextInstructionDecoderState == instructionDecoderState) {
			running = false;
		}
		instructionDecoderState = nextInstructionDecoderState;
	}

	void MinimalMachine::emulateInstructionCycle() {
		//keep emulating clock cycles until the instruction zero is reached or the MiMa is halted
		do {
			emulateClockCycle();
		} while (instructionDecoderState != 0 && running);
	}

	void MinimalMachine::emulateLifeTime() {
		//keep emulating clock cycles until the MiMa is halted
		while (running) {
			emulateClockCycle();
		}
	}


	void MinimalMachine::printState(std::ostream& output) const {
		output << "MinimalMachine state:" << std::endl;

		//Instruction decoder state:
		structure(0, 0, false, output);
		output << "Decoder state" << std::endl;
		
		structure(0, 1, false, output);
		output << "Next regTransfer: " << (int)instructionDecoderState << std::endl;
		structure(0, 1, true, output);
		output << "Running: " << trueFalse(running) << std::endl;

		structureSkip(0, 1, output);

		//Registers:
		structure(0, 0, true, output);
		output << "Registers" << std::endl;

		structure(1, 0, false, output);
		output << "Accumulator: " << accumulator.value << " (negative: " << trueFalse(accumulator.negative.value) << ")" << std::endl;
		structure(1, 0, false, output);
		output << "IAR: " << instructionAddressRegister << std::endl;
		structure(1, 0, false, output);
		output << "IR: " << instructionRegister.value << std::endl;
		structure(1, 0, false, output);
		output << "X: " << X << std::endl;
		structure(1, 0, false, output);
		output << "Y: " << Y << std::endl;
		structure(1, 0, false, output);
		output << "Z: " << Z << std::endl;
		structure(1, 0, false, output);
		output << "SAR: " << storageAddressRegister << std::endl;
		structure(1, 0, true, output);
		output << "SDR: " << storageDataRegister << std::endl;

		structureSkip(0, 0, output);
	}
}
