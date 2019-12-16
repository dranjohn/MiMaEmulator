#include <iostream>

#include "mima/MinimalMachine.h"

MiMa::DecodeAction decodeInstructionJump(const uint8_t&, const uint8_t& opCode, uint8_t& instructionDecoderState) {
	uint8_t shortOpCode = (opCode & 0xF0) >> 4;
	if (shortOpCode == 0xF) { //use extended opCode

	}
	else { //use shortOpCode
		switch(shortOpCode) {
		case 0:
			instructionDecoderState = 7;
			break;
		}
	}

	return MiMa::DecodeAction::SKIP_CYCLE;
}

int main() {
	uint32_t* instructionDecoder = new uint32_t[32];
	MiMa::MemoryCell* memory = new MiMa::MemoryCell[MiMa::MinimalMachine::MEMORY_CAPACITY];
	memory[0] = { 0x000000FF };

	instructionDecoder[0] = 0x02108801; //fetch cycle
	instructionDecoder[1] = 0x01400802;
	instructionDecoder[2] = 0x00001803;
	instructionDecoder[3] = 0x00A00004;
	instructionDecoder[4] = 0x00090005;
	instructionDecoder[5] = 0x10000000; //decode instruction
	instructionDecoder[6] = 0x01800000; //return Z to acc
	instructionDecoder[7] = 0x08040000; //LDC

	MiMa::MinimalMachine mima(instructionDecoder, decodeInstructionJump, memory);
	mima.printState(std::cout);

	mima.emulateInstructionCycle();
	mima.printState(std::cout);

	delete[] instructionDecoder;
	delete[] memory;

	return 0;
}
