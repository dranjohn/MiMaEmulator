#pragma once

#include <cstdint>

namespace MiMa {
	constexpr size_t DEFAULT_MEMORY_CAPACITY = 0xFFFFF;

	struct MemoryCell {
		uint32_t data : 24;
		uint32_t debug : 8;

		MemoryCell(uint32_t dataValue = 0) : data(dataValue), debug(0) {}
	};
}
