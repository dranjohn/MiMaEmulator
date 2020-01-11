#include <iostream>
#include <memory>
#include <vector>

#include "mima/microprogram/MicroProgram.h"
#include "mima/microprogram/MicroProgramCompiler.h"
#include "mima/MinimalMachine.h"
#include "debug/Log.h"


int main() {
	std::unique_ptr<MiMa::MemoryCell[]> memory = std::make_unique<MiMa::MemoryCell[]>(MiMa::MinimalMachine::MEMORY_CAPACITY);

	char* instructionDecoderCode =
		"//fetch phase!\n"
		"start: IAR > SAR; IAR > X; R = 1;;\n"
		"ONE > Y; R = 1;;\n"
		"ALU = ADD; R = 1;;\n"
		"Z > IAR;;\n"
		"SDR > IR;;\n\n"

		"//conditional decode by the first eight bits of the instruction register!\n"
		"cm(conditional, op_code, 255)!\n"
		"//default decode result is halting!\n"
		"#halt;\n"
		"//short operation codes (span from 0xX0 to 0xXF)!\n"
		"0+ 15- #ldc;\n"
		"16+ 31- #ldv;\n"
		"32+ 47- #stv;\n"
		"48+ 63- #add;\n"
		"64+ 79- #and;\n"
		"80+ 95- #or;\n"
		"96+ 111- #xor;\n"
		"//long operation codes (one of 0xFX)!\n"
		"112+ 127- #eql;\n"
		"128+ 143- #jmp;\n"
		"144+ 159- #jmn;\n"
		"241= #not;\n"
		"242= #rar;\n"
		";\n\n"

		"cm(default)!\n"
		"//return function (often used, therefore only once)!\n"
		"ret: Z > ACCU; #start;;\n\n"

		"//load a constant to accumulator!\n"
		"ldc: IR > ACCU; #start;;\n\n"

		"//load a value from storage to accumulator!\n"
		"ldv: IR > SAR; R = 1;;\n"
		"R = 1;;\n"
		"R = 1;;\n"
		"SDR > ACCU; #start;;\n\n"

		"//store the value in the accumulator to storage!\n"
		"stv: IR > SAR;;\n"
		"ACCU > SDR; W = 1;;\n"
		"W = 1;;\n"
		"W = 1; #start;;\n\n"

		"//add the value in the accumulator and one in storage!\n"
		"add: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = ADD; #ret;;\n\n"

		"//apply bitwise and to the value in the accumulator and one in storage!\n"
		"and: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = AND; #ret;;\n\n"

		"//apply bitwise or to the value in the accumulator and one in storage!\n"
		"or: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = OR; #ret;;\n\n"

		"//apply bitwise xor to the value in the accumulator and one in storage!\n"
		"xor: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = XOR; #ret;;\n\n"

		"//compare the value in the accumulator and one in storage!\n"
		"//store ~0 in accumulator if they are equal, 0 otherwise!\n"
		"eql: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = EQL; #ret;;\n\n"
		
		"//jump to the given instruction!\n"
		"jmp: IR > IAR; #start;;\n\n"

		"//jump only if the value in the accumulator is negative (for example ~0)!\n"
		"jmn:;;\n"
		"cm(conditional, accumulator_negative, 1)!\n"
		"0= #start;\n"
		"1= #jmp;\n"
		";\n\n"

		"//halt at 0xF0 is implicitly defined, since halting is the default action!\n\n"

		"//apply bitwise not to the value in the accumulator!\n"
		"cm(default)!"
		"not: IR > SAR; R = 1;;\n"
		"ACCU > X; R = 1;;\n"
		"R = 1;;\n"
		"SDR > Y; ALU = NOT; #ret;;\n\n"

		"//apply bitwise right rotation to the value in the accumulator!\n"
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
