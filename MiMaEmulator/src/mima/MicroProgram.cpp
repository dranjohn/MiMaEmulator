#include "MicroProgram.h"

#include "StatusBit.h"
#include "debug/Log.h"
#include "MicroProgramCompiler.h"

#include <vector>
#include <fstream>
#include <limits>
#include <cctype>

namespace MiMa {
	MicroProgram::MicroProgram(const uint8_t& startingPoint) : memory(new uint32_t[256]) , firstFree(startingPoint) {
		MIMA_LOG_INFO("Created microprogram starting at 0x{:02X}", startingPoint);
		MIMA_LOG_WARN("Using the microprogram compiler is not thread-safe!");
	}


	//Interface: read input from a pointer to a char array containing the code for the program.
	void MicroProgram::compile(char* microProgramCode) {
		MIMA_LOG_INFO("Compiling microprogram from given code string");

		BufferedCharStream microProgramCodeStream(microProgramCode);
		compile(microProgramCodeStream);
	}

	//Interface: read input from an inputstream providing the code for the program.
	void MicroProgram::compile(std::istream& microProgramCode) {
		MIMA_LOG_INFO("Starting microprogram compilation");
		
		InputCharStream microProgramCodeStream(microProgramCode);
		compile(microProgramCodeStream);
	}


	//Interface: read input from a file containing the code for the program.
	void MicroProgram::compileFile(const char* fileName) {
		MIMA_LOG_INFO("Compiling microprogram from file '{}'", fileName);

		std::ifstream fileInput;
		fileInput.open(fileName);

		compile(fileInput);

		fileInput.close();
	}


	void MicroProgram::compile(CharStream& microProgramCodeStream) {
		//initialize token buffer
		std::vector<char> tokenBuffer;

		//initialize micro code builder
		MicroProgramCompiler codeBuilder(memory, firstFree, labels, unresolvedLabels);
		char input;

		//check for control token
		while (input = microProgramCodeStream.get()) {
			if (isspace(input)) {
				continue;
			}

			//control character terminate tokens
			if (codeBuilder.isControl(input)) {
				tokenBuffer.push_back(0);

				MIMA_LOG_TRACE("Found microprogram token: '{}'", tokenBuffer.data());
				MIMA_LOG_TRACE("Found microprogram control character: '{}'", input);

				codeBuilder.addToken(input, tokenBuffer.data());

				tokenBuffer.clear();
				continue;
			}

			//token not terminated, add current character to the token
			tokenBuffer.push_back(input);
		}

		//add final token
		tokenBuffer.push_back(0);
		codeBuilder.finish(tokenBuffer.data());
	}


	std::shared_ptr<uint32_t[]> MicroProgram::getMemory() {
		MIMA_ASSERT_WARN(unresolvedLabels.empty(), "Microprogram memory with unresolved labels has been requested");
		return memory;
	}


	void MicroProgram::printLabels() const {
		for (auto const& [label, loc] : labels) {
			MIMA_LOG_INFO("Found label: {} at 0x{:02X}", label, loc);
		}
	}

	void MicroProgram::printCompileStart() const {
		MIMA_LOG_INFO("Compilation start is at 0x{:02X}", firstFree);
	}
}
