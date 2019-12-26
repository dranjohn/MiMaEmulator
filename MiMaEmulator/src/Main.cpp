#include <iostream>
#include <memory>
#include <vector>

#include "mima/MicroProgram.h"
#include "mima/MinimalMachine.h"
#include "debug/Log.h"


uint32_t decodeInstructionJump(const uint8_t&, const uint8_t& opCode) {
	uint8_t shortOpCode = (opCode & 0xF0) >> 4;
	
	uint32_t nextAddress = 0;
	if (shortOpCode == 0xF) { //use extended opCode
		uint8_t opCodeExtension = opCode & 0xF;

		switch (opCodeExtension) {
		case 0:
			return 0xFF;
		}
	}
	else { //use shortOpCode
		switch(shortOpCode) {
		case 0:
			nextAddress = 0x07;
			break;
		case 1:
			nextAddress = 0x08;
			break;
		case 2:
			nextAddress = 0x0C;
			break;
		case 3:
			nextAddress = 0x10;
			break;
		case 4:
			nextAddress = 0x14;
			break;
		}
	}

	return nextAddress;
}

int main() {
	std::unique_ptr<MiMa::MemoryCell[]> memory = std::make_unique<MiMa::MemoryCell[]>(MiMa::MinimalMachine::MEMORY_CAPACITY);

	char* instructionDecoderCode =
		"start: IAR > SAR; IAR > X; R = 1;;\n"
		"ONE > Y; R = 1;;\n"
		"ALU = ADD; R = 1;;\n"
		"Z > IAR;;\n"
		"SDR > IR;;\n"
		"D = 1;;\n"
		"ret: Z > ACCU; #start;;\n"
		"ldc: IR > ACCU; #start;;\n"
		"ldv: IR > SAR; R = 1;;\n"
		"R = 1;;\n"
		"R = 1;;\n"
		"SDR > ACCU; #start;;\n"
		"stv: IR > SAR;;\n"
		"ACCU > SDR; W = 1;;\n"
		"W = 1;;\n"
		"W = 1; #start;;\n"
		"add: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = ADD; #ret;;\n"
		"and: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = AND; #ret;;\n";
	MiMa::MicroProgram microprogram;
	microprogram.compile(instructionDecoderCode);
	std::shared_ptr<uint32_t[]> instructionDecoder = microprogram.getMemory();

	memory[0x00] = { 0x000000FF };
	memory[0x01] = { 0x00300020 };
	memory[0x02] = { 0x00F00000 };
	memory[0x20] = { 0x00000003 };

	MiMa::MinimalMachine mima(instructionDecoder, decodeInstructionJump, memory.get());
	mima.printState();
	
	mima.emulateLifeTime();
	mima.printState();
	
	return 0;
}
