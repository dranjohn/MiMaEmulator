#include <iostream>
#include <memory>

#include "mima/MinimalMachine.h"

MiMa::DecodeAction decodeInstructionJump(const uint8_t&, const uint8_t& opCode, uint8_t& instructionDecoderState) {
	uint8_t shortOpCode = (opCode & 0xF0) >> 4;
	if (shortOpCode == 0xF) { //use extended opCode

	}
	else { //use shortOpCode
		switch(shortOpCode) {
		case 0:
			instructionDecoderState = 0x07;
			break;
		case 1:
			instructionDecoderState = 0x08;
			break;
		case 2:
			instructionDecoderState = 0x0C;
			break;
		case 3:
			instructionDecoderState = 0x10;
			break;
		case 4:
			instructionDecoderState = 0x14;
			break;
		}
	}

	return MiMa::DecodeAction::SKIP_CYCLE;
}

int main() {
	std::unique_ptr<uint32_t[]> instructionDecoder = std::make_unique<uint32_t[]>(256);
	std::unique_ptr<MiMa::MemoryCell[]> memory = std::make_unique<MiMa::MemoryCell[]>(MiMa::MinimalMachine::MEMORY_CAPACITY);

	instructionDecoder[0x00] = 0x02108801; //fetch cycle
	instructionDecoder[0x01] = 0x01400802;
	instructionDecoder[0x02] = 0x00001803;
	instructionDecoder[0x03] = 0x00A00004;
	instructionDecoder[0x04] = 0x00090005;
	instructionDecoder[0x05] = 0x10000000; //decode instruction
	instructionDecoder[0x06] = 0x08800000; //return Z to acc
	instructionDecoder[0x07] = 0x08040000; //LDC
	instructionDecoder[0x08] = 0x00048809; //LDV
	instructionDecoder[0x09] = 0x0000080A;
	instructionDecoder[0x0A] = 0x0000080B;
	instructionDecoder[0x0B] = 0x08010000;
	instructionDecoder[0x0C] = 0x0004800D; //STV
	instructionDecoder[0x0D] = 0x0402040E;
	instructionDecoder[0x0E] = 0x0000040F;
	instructionDecoder[0x0F] = 0x00000400;
	instructionDecoder[0x10] = 0x00048811; //ADD
	instructionDecoder[0x11] = 0x06000812;
	instructionDecoder[0x12] = 0x00000813;
	instructionDecoder[0x13] = 0x01011006; //return ADD
	instructionDecoder[0x14] = 0x00048815; //AND
	instructionDecoder[0x15] = 0x06000816;
	instructionDecoder[0x16] = 0x00000817;
	instructionDecoder[0x17] = 0x01013006; //return AND

	memory[0x00] = { 0x000000FF };
	memory[0x01] = { 0x00300020 };
	memory[0x20] = { 0x00000003 };

	MiMa::MinimalMachine mima(instructionDecoder.get(), decodeInstructionJump, memory.get());
	mima.printState(std::cout);

	for (int i = 0; i < 2; i++) {
		mima.emulateInstructionCycle();
		mima.printState(std::cout);
	}

	return 0;
}
