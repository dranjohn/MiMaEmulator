#pragma once

#include <regex>
#include <string>

#include "mima/MinimalMachine.h"

namespace MiMa {
	class MiMaMemoryCompiler {
	private:
		static const std::regex assignmentMatcher;
		static const std::regex instructionMatcher;

		static const std::regex identifiers;
		static const std::regex decNumber;
		static const std::regex hexNumber;

	private:
		uint32_t compilationAddress = 0;
		MemoryCell* mimaMemory = new MemoryCell[MEMORY_CAPACITY];

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
