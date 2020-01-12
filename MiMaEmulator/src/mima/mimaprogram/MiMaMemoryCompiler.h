#pragma once

#include <regex>
#include <string>

#include "mima/MinimalMachine.h"

namespace MiMa {
	class MiMaMemoryCompiler {
	private:
		const static std::regex assignmentMatcher;
		const static std::regex instructionMatcher;

		const static std::regex identifiers;
		const static std::regex decNumber;
		const static std::regex hexNumber;

	private:
		uint32_t compilationAddress = 0;
		MemoryCell* mimaMemory = new MemoryCell[MinimalMachine::MEMORY_CAPACITY];

	private:
		bool addFunction(const std::string& functionName);
		bool addUnaryFunction(const std::string& functionName, const uint32_t& argument);

	public:
		void addLine(const std::string& line);

		void finish();

		// ------------------------------------------------------
		// Compilation utility methods
		// Use these for simple compilation of common input types
		// ------------------------------------------------------
		static MemoryCell* compile(char* mimaProgramCode);
	};
}
