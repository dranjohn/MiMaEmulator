#include <iostream>
#include <memory>
#include <vector>
#include <sstream>

#include "mima/MicroProgram.h"
#include "mima/MinimalMachine.h"

MiMa::DecodeAction decodeInstructionJump(const uint8_t&, const uint8_t& opCode, uint8_t& instructionDecoderState) {
	uint8_t shortOpCode = (opCode & 0xF0) >> 4;
	
	if (shortOpCode == 0xF) { //use extended opCode
		uint8_t opCodeExtension = opCode & 0xF;

		switch (opCodeExtension) {
		case 0:
			return MiMa::DecodeAction::HALT;
		}
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

	char* input =
		"start: IAR > SAR; IAR > X; R = 1;;"
		"ONE > Y; R = 1;;"
		"ALU = ADD; R = 1;;"
		"Z > IAR;;"
		"SDR > IR;;"
		"D = 1;;"
		"ret: Z > ACCU; #start;;"
		"ldc: IR > ACCU; #start;;"
		"ldv: IR > SAR; R = 1;;"
		"R = 1;;"
		"R = 1;;"
		"SDR > ACCU; #start;;"
		"stv: IR > SAR;;"
		"ACCU > SDR; W = 1;;"
		"W = 1;;"
		"W = 1; #start;;"
		"add: IR > SAR; R = 1;;"
		"ACCU > X; R = 1;;"
		"R = 1;;"
		"SDR > Y; ALU = ADD; #ret;;"
		"and: IR > SAR; R = 1;;"
		"ACCU > X; R = 1;;"
		"R = 1;;"
		"SDR > Y; ALU = AND; #ret;;";

	MiMa::readInput(instructionDecoder.get(), input);

	memory[0x00] = { 0x000000FF };
	memory[0x01] = { 0x00300020 };
	memory[0x02] = { 0x00F00000 };
	memory[0x20] = { 0x00000003 };

	MiMa::MinimalMachine mima(instructionDecoder.get(), decodeInstructionJump, memory.get());
	mima.printState(std::cout);
	
	mima.emulateLifeTime();
	mima.printState(std::cout);

	return 0;
}
