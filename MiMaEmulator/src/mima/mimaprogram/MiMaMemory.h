#pragma once

#include <cstdint>
#include <memory>

namespace MiMa {
	constexpr size_t DEFAULT_MEMORY_CAPACITY = 0xFFFFF;

	struct MemoryCell {
		uint32_t data : 24;
		uint32_t debug : 8;

		MemoryCell(uint32_t dataValue = 0) : data(dataValue), debug(0) {}
	};


	class MiMaMemory {
	private:
		std::unique_ptr<MemoryCell[]> memory;
	public:
		MiMaMemory();

		inline MemoryCell& operator[](size_t index) { return memory[index]; }
		inline const MemoryCell& operator[] (size_t index) const { return memory[index]; }
	};
}
