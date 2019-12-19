#pragma once

#include <cstdint>
#include <istream>

namespace MiMa {
	void readInput(uint32_t* memory, char* microProgramCode, uint8_t startingPoint = 0);
	void readInput(uint32_t* memory, std::istream& microProgramCode, uint8_t startingPoint = 0);

	void readFile(uint32_t* memory, const char* fileName, uint8_t startingPoint = 0);
}
