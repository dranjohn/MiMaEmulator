#pragma once

#include <cstddef>

#define BIT(n) (1 << n)

namespace MiMa {
	namespace StatusBit {
		constexpr uint32_t FOLLOWING_ADDRESS = 0xFF;
		constexpr uint32_t RESERVED = 0x300;
		constexpr uint32_t STORAGE_WRITING = BIT(10);
		constexpr uint32_t STORAGE_READING = BIT(11);
		constexpr uint32_t ALU_C = 0x7000;
		constexpr uint32_t SAR_READING = BIT(15);
		constexpr uint32_t SDR_WRITING = BIT(16);
		constexpr uint32_t SDR_READING = BIT(17);
		constexpr uint32_t IR_WRITING = BIT(18);
		constexpr uint32_t IR_READING = BIT(19);
		constexpr uint32_t IAR_WRITING = BIT(20);
		constexpr uint32_t IAR_READING = BIT(21);
		constexpr uint32_t ONE = BIT(22);
		constexpr uint32_t ALU_RESULT = BIT(23);
		constexpr uint32_t ALU_RIGHT_OPERAND = BIT(24);
		constexpr uint32_t ALU_LEFT_OPERAND = BIT(25);
		constexpr uint32_t ACCUMULATOR_WRITING = BIT(26);
		constexpr uint32_t ACCUMULATOR_READING = BIT(27);
		constexpr uint32_t UNDEFINED = 0xF0000000;
	}
}
