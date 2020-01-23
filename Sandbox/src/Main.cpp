//std library
#include <iostream>
#include <memory>
#include <vector>

//minimal machine
#include "mima/microprogram/MicroProgram.h"
#include "mima/microprogram/MicroProgramCompiler.h"
#include "mima/mimaprogram/MiMaCompiler.h"
#include "mima/mimaprogram/MiMaMemory.h"
#include "mima/MinimalMachine.h"
#include "mima/CompilerException.h"

//debugging utility
#include "debug/Log.h"


int main() {
	std::shared_ptr<const MiMa::MicroProgram> instructionDecoder = MiMa::MicroProgramCompiler::compileFile("idCode.txt");
	std::shared_ptr<MiMa::MiMaMemory> memory = MiMa::MiMaMemoryCompiler::compileFile("mimaProgram.txt");

	MiMa::MinimalMachine mima(instructionDecoder, memory);

	//run the mima
	MIMA_LOG_INFO("\n{}", mima);

	mima.emulateLifeTime();
	MIMA_LOG_INFO("\n{}", mima);

	const MiMa::MiMaMemory& memRef = *memory;
	MIMA_LOG_INFO("Minimal machine memory state:\n{}", memRef);

	return 0;
}
