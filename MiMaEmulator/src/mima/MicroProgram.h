#pragma once

#include <cstdint>
#include <istream>
#include <memory>
#include <string>
#include <map>

#include "util/CharStream.h"

namespace MiMa {
	class MicroProgram {
	private:
		//program memory
		std::shared_ptr<uint32_t[]> memory;
		uint8_t firstFree;

		//labels
		std::map<std::string, uint8_t> labels;
		std::multimap<std::string, uint8_t> unresolvedLabels;
	public:
		MicroProgram(const uint8_t& startingPoint = 0);

		//compile code and add it to memory
		void compile(char* microProgramCode);
		void compile(std::istream& microProgramCode);

		void compileFile(const char* fileName);
	private:
		void compile(CharStream& microProgramCodeStream);

	public:
		//gets program memory
		std::shared_ptr<uint32_t[]> getMemory();

		//debug functions
		void printLabels() const;
		void printCompileStart() const;
	};
}
