#include <iostream>
#include <memory>
#include <vector>

#include "mima/microprogram/MicroProgram.h"
#include "mima/microprogram/MicroProgramCompiler.h"
#include "mima/mimaprogram/MiMaMemoryCompiler.h"
#include "mima/MinimalMachine.h"
#include "debug/Log.h"


int main() {
	char* instructionDecoderCode =
		"//fetch phase\n"
		"start: IAR -> SAR; IAR -> X; R = 1;\n"
		"ONE -> Y; R = 1;\n"
		"ALU = ADD; R = 1;\n"
		"Z -> IAR;\n"
		"SDR -> IR;\n\n"

		"//conditional decode by the first eight bits of the instruction register\n"
		"!cm(conditional, op_code, 255)\n"
		"//default decode result is halting\n"
		"[0, max] #halt\n"
		"//short operation codes (span from 0xX0 to 0xXF)\n"
		"[0, 15] #ldc\n"
		"[16, 31] #ldv\n"
		"[32, 47] #stv\n"
		"[48, 63] #add\n"
		"[64, 79] #and\n"
		"[80, 95] #or\n"
		"[96, 111] #xor\n"
		"[112, 127] #eql\n"
		"[128, 143] #jmp\n"
		"[144, 159] #jmn\n"
		"//long operation codes (one of 0xFX)\n"
		"//[240, 240] #halt is implicitly defined, since halting is the default\n"
		"[241, 241] #not\n"
		"[242, 242] #rar;\n\n"

		"!cm(default)\n\n"

		"//return function (often used, therefore declaration here)\n"
		"ret: Z -> ACCU; #start;\n\n"

		"//load a constant to accumulator\n"
		"ldc: IR -> ACCU; #start;\n\n"

		"//load a value from storage to accumulator\n"
		"ldv: IR -> SAR; R = 1;\n"
		"R = 1;\n"
		"R = 1;\n"
		"SDR -> ACCU; #start;\n\n"

		"//store the value in the accumulator to storage\n"
		"stv: IR -> SAR;\n"
		"ACCU -> SDR; W = 1;\n"
		"W = 1;\n"
		"W = 1; #start;\n\n"

		"//add the value in the accumulator and one in storage\n"
		"add: IR -> SAR; R = 1;\n"
		"ACCU -> X; R = 1;\n"
		"R = 1;\n"
		"SDR -> Y; ALU = ADD; #ret;\n\n"

		"//apply bitwise and to the value in the accumulator and one in storage\n"
		"and: IR -> SAR; R = 1;\n"
		"ACCU -> X; R = 1;\n"
		"R = 1;\n"
		"SDR -> Y; ALU = AND; #ret;\n\n"

		"//apply bitwise or to the value in the accumulator and one in storage\n"
		"or: IR -> SAR; R = 1;\n"
		"ACCU -> X; R = 1;\n"
		"R = 1;\n"
		"SDR -> Y; ALU = OR; #ret;\n\n"

		"//apply bitwise xor to the value in the accumulator and one in storage\n"
		"xor: IR -> SAR; R = 1;\n"
		"ACCU -> X; R = 1;\n"
		"R = 1;\n"
		"SDR -> Y; ALU = XOR; #ret;\n\n"

		"//compare the value in the accumulator and one in storage\n"
		"//store ~0 in accumulator if they are equal, 0 otherwise\n"
		"eql: IR -> SAR; R = 1;\n"
		"ACCU -> X; R = 1;\n"
		"R = 1;\n"
		"SDR -> Y; ALU = EQL; #ret;\n\n"

		"//jump to the given instruction!\n"
		"jmp: IR -> IAR; #start;\n\n"

		"//jump only if the value in the accumulator is negative (for example ~0)\n"
		"jmn:\n"
		"!cm(conditional, accumulator_negative, 1)\n"
		"[0, 0] #start\n"
		"[1, 1] #jmp;\n\n"

		"!cm(default)\n\n"

		"//apply bitwise not to the value in the accumulator\n"
		"not: ACCU -> X; ALU = NOT; #ret;\n\n"

		"//apply bitwise right rotation to the value in the accumulator\n"
		"rar: ACCU -> X; ALU = RAR; #ret;\n\n";

	//create instruction decoder from given code
	MiMa::MicroProgram instructionDecoder = MiMa::MicroProgramCompiler::compile(instructionDecoderCode);
	
	//write mima program into its memory
	char* programCode =
		"* = 0\n"
		"LDC $FF\n"
		"ADD $20\n"
		"HALT\n"
		"* = $20\n"
		"DS 3\n";
	MiMa::MemoryCell* memory = MiMa::MiMaMemoryCompiler::compile(programCode);

	//run the mima
	MiMa::MinimalMachine mima(instructionDecoder, memory);
	mima.printState();

	mima.emulateLifeTime();
	mima.printState();

	delete[] memory;

	return 0;
}
