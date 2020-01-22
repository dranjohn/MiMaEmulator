#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include "util/BinarySearchTree.h"

namespace MiMa {
	constexpr size_t DEFAULT_MEMORY_CAPACITY = 0xFFFFF;

	struct MemoryCell {
		uint32_t data : 24;
		uint32_t debug : 8;

		MemoryCell(uint32_t dataValue = 0) : data(dataValue), debug(0) {}
	};


	class MiMaMemory {
	private:
		class MemoryBlock {
		public:
			static const size_t MEMORY_BLOCK_SIZE = 256;

		private:
			MemoryCell memory[MEMORY_BLOCK_SIZE];

		public:
			inline MemoryCell& operator[](const size_t& index) { return memory[index]; };
			const inline MemoryCell& operator[](const size_t& index) const { return memory[index]; };
		};


	private:
		static const std::function<std::shared_ptr<MemoryBlock>(void)> emptyMemoryBlockConstructor;

	private:
		BinarySearchTree<MemoryBlock> memory = BinarySearchTree(0, emptyMemoryBlockConstructor);

	public:
		MemoryCell& operator[](const size_t& index);
	};
}
