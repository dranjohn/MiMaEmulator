#pragma once

#include <cstdint>
#include <memory>

#include "util/CharStream.h"

namespace MiMa {
	class MicroProgramCode {
	private:
		uint32_t bits;

	public:
		MicroProgramCode(const uint32_t& bits = 0) : bits(bits) {}

		uint32_t getBits(const uint8_t& opCode) const { return bits; };
	};

	class MicroProgram {
	private:
		//program memory
		std::shared_ptr<MicroProgramCode[]> memory;
	public:
		MicroProgram(const std::shared_ptr<MicroProgramCode[]>& memory) : memory(memory) {}

		inline uint32_t getMicroCode(const uint8_t& memoryAddress, const uint8_t& opCode) const { return memory[memoryAddress].getBits(opCode); }
	};
}
