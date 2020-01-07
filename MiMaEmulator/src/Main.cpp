#include <iostream>
#include <memory>
#include <vector>

#include "mima/MicroProgram.h"
#include "mima/MicroProgramCompiler.h"
#include "mima/MinimalMachine.h"
#include "debug/Log.h"


int main() {
	std::unique_ptr<MiMa::MemoryCell[]> memory = std::make_unique<MiMa::MemoryCell[]>(MiMa::MinimalMachine::MEMORY_CAPACITY);

	char* instructionDecoderCode =
		"start: IAR > SAR; IAR > X; R = 1;;\n"
		"ONE > Y; R = 1;;\n"
		"ALU = ADD; R = 1;;\n"
		"Z > IAR;;\n"
		"SDR > IR;;\n"

		"cm_conditional!\n"
		"#halt;\n"
		"0+ 15- #ldc;\n"
		"16+ 31- #ldv;\n"
		"32+ 47- #stv;\n"
		"48+ 63- #add;\n"
		"64+ 79- #and;\n"
		";\n"

		"cm_default!\n"
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
	MiMa::MicroProgram instructionDecoder = MiMa::MicroProgramCompiler::compile(instructionDecoderCode);
	for (int i = 0; i < 0x19; i++) {
		MIMA_LOG_INFO("Compiled code at 0x{:02X}: {}", i, instructionDecoder.memory[i]);
	}

	memory[0x00] = { 0x0000FF };
	memory[0x01] = { 0x300020 };
	memory[0x02] = { 0xF00000 };
	memory[0x20] = { 0x000003 };

	MiMa::MinimalMachine mima(instructionDecoder, memory.get());
	mima.printState();

	mima.emulateLifeTime();
	mima.printState();

	return 0;
}
