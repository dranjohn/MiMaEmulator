#include <iostream>
#include <memory>
#include <vector>

#include "mima/microprogram/MicroProgram.h"
#include "mima/microprogram/MicroProgramCompiler.h"
#include "mima/mimaprogram/MiMaMemoryCompiler.h"
#include "mima/MinimalMachine.h"
#include "debug/Log.h"
#include "mima/CompilerException.h"


int main() {
	//create instruction decoder from given code
	//MiMa::MicroProgram instructionDecoder; = MiMa::MicroProgramCompiler::compile(instructionDecoderCode);
	try {
		MiMa::MicroProgram instructionDecoder = MiMa::MicroProgramCompiler::compileFile("idCode.txt");
	}
	catch (const MiMa::CompilerException& exc) {
		MIMA_LOG_CRITICAL(exc.what());
		return 0;
	}
	
	//write mima program into its memory
	char* programCode =
		"* = 0\n"
		"LDC $FF\n"
		"ADD $20\n"
		"HALT\n"
		"* = $20\n"
		"DS 3\n";

	try {
		MiMa::MemoryCell* memory = MiMa::MiMaMemoryCompiler::compileFile("mimaProgram.txt");
		delete[] memory;
	}
	catch (const MiMa::CompilerException & exc) {
		MIMA_LOG_CRITICAL(exc.what());
		return 0;
	}

	//run the mima
	/*MiMa::MinimalMachine mima(instructionDecoder, memory);
	mima.printState();

	mima.emulateLifeTime();
	mima.printState();*/

	return 0;
}
