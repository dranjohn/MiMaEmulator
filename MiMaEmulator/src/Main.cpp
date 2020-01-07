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
		"SDR > IR;;\n\n"

		"cm(conditional)!\n"
		"#halt;\n"
		"0+ 15- #ldc;\n"
		"16+ 31- #ldv;\n"
		"32+ 47- #stv;\n"
		"48+ 63- #add;\n"
		"64+ 79- #and;\n"
		"80+ 95- #or;\n"
		"96+ 111- #xor;\n"
		"112+ 127- #eql;\n"
		"128+ 143- #jmp;\n"
		"144+ 159- #jmn;\n"
		"241= #not;\n"
		"242= #rar;\n"
		";\n\n"

		"cm(default)!\n"
		"ret: Z > ACCU; #start;;\n\n"

		"ldc: IR > ACCU; #start;;\n\n"

		"ldv: IR > SAR; R = 1;;\n"
		"R = 1;;\n"
		"R = 1;;\n"
		"SDR > ACCU; #start;;\n\n"

		"stv: IR > SAR;;\n"
		"ACCU > SDR; W = 1;;\n"
		"W = 1;;\n"
		"W = 1; #start;;\n\n"

		"add: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = ADD; #ret;;\n\n"

		"and: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = AND; #ret;;\n\n"
		
		"or: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = OR; #ret;;\n\n"
		
		"xor: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = XOR; #ret;;\n\n"
		
		"eql: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = EQL; #ret;;\n\n"
		
		"jmp: IR > IAR; #start;;\n\n"
		
		//jmn not working yet

		"not: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = NOT; #ret;;\n\n"
		
		"rar: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = RAR; #ret;;\n\n";


	//create instruction decoder from given code
	MiMa::MicroProgram instructionDecoder = MiMa::MicroProgramCompiler::compile(instructionDecoderCode);
	
	//write mima program into its memory
	memory[0x00] = { 0x0000FF };
	memory[0x01] = { 0x300020 };
	memory[0x02] = { 0xF00000 };
	memory[0x20] = { 0x000003 };

	//run the mima
	MiMa::MinimalMachine mima(instructionDecoder, memory.get());
	mima.printState();

	mima.emulateLifeTime();
	mima.printState();

	return 0;
}
