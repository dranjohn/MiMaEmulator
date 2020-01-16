#include <iostream>
#include <memory>
#include <vector>

#include "mima/microprogram/MicroProgram.h"
#include "mima/microprogram/MicroProgramCompiler.h"
#include "mima/mimaprogram/MiMaCompiler.h"
#include "mima/MinimalMachine.h"
#include "debug/Log.h"
#include "mima/CompilerException.h"


int main() {
	std::shared_ptr<const MiMa::MicroProgram> instructionDecoder = MiMa::MicroProgramCompiler::compileFile("idCode.txt");
	std::shared_ptr<MiMa::MemoryCell[]> memory = MiMa::MiMaMemoryCompiler::compileFile("mimaProgram.txt");

	MiMa::MinimalMachine mima(instructionDecoder, memory);

	//run the mima
	MIMA_LOG_INFO(mima);

	mima.emulateLifeTime();
	MIMA_LOG_INFO(mima);

	return 0;
}
