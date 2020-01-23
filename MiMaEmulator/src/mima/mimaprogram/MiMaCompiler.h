#pragma once

//std library
#include <fstream>
#include <istream>
#include <memory>
#include <regex>
#include <string>

//internal classes
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
		std::shared_ptr<MiMaMemory> mimaMemory = std::make_shared<MiMaMemory>();

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
		static std::shared_ptr<MiMaMemory> compile(const std::string& mimaProgramCode);
		static std::shared_ptr<MiMaMemory> compile(std::istream& mimaProgramCode);

		static std::shared_ptr<MiMaMemory> compileFile(const std::string& fileName);
	};
}
