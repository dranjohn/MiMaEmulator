#pragma once

//std library
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

//external vendor libraries
#include <fmt/format.h>

//internal utility
#include "util/BinarySearchTree.h"

//debugging utility
#include "debug/Log.h"


namespace MiMa {
	constexpr size_t DEFAULT_MEMORY_CAPACITY = 0xFFFFF;

	struct MemoryCell {
		friend struct fmt::formatter<MiMa::MemoryCell>;

		uint32_t data : 24;
		uint32_t debug : 8;

		MemoryCell(uint32_t dataValue = 0) : data(dataValue), debug(0) {}

		inline bool operator==(const MemoryCell& other) const { return data == other.data; }
		inline bool operator!=(const MemoryCell& other) const { return data != other.data; }
	};


	class MiMaMemory {
		friend struct fmt::formatter<MiMa::MiMaMemory>;

	private:
		class MemoryBlock {
			friend MemoryBlock;

		public:
			static const size_t MEMORY_BLOCK_SIZE = 0x10;

		private:
			MemoryCell memory[MEMORY_BLOCK_SIZE];

		public:
			bool operator==(const MemoryBlock& other) const {
				for (size_t i = 0; i < MEMORY_BLOCK_SIZE; ++i) {
					if (memory[i] != other.memory[i]) {
						return false;
					}
				}
				return true;
			}
			inline bool operator!=(const MemoryBlock& other) const { return !(*this == other); }

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



//minimal machine memory fmt formatting definition
template<>
struct fmt::formatter<MiMa::MemoryCell> {
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

	template<typename FormatContext>
	auto format(const MiMa::MemoryCell& memoryCell, FormatContext& ctx) {
		if (memoryCell.debug == 0) {
			return fmt::format_to(ctx.out(), "{:06X}", memoryCell.data);
		}

		return fmt::format_to(ctx.out(), "{:06X} (debug value {:02X})", memoryCell.data, memoryCell.debug);
	}
};

template<>
struct fmt::formatter<MiMa::MiMaMemory> {
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

	template<typename FormatContext>
	auto format(const MiMa::MiMaMemory& memory, FormatContext& ctx) {
		BinarySearchTreeIterator<MiMa::MiMaMemory::MemoryBlock> it(memory.memory);
		BinarySearchTreeIterator<MiMa::MiMaMemory::MemoryBlock> end;

		std::string treeOutput = "Minimal machine memory:\n{}";
		std::string blockFormat = "from {:X}:\n{}";
		std::string addressFormat = "{}\n{{}}";

		while (it != end) {
			std::pair<size_t, std::shared_ptr<MiMa::MiMaMemory::MemoryBlock>> currentBlock = *it;
			
			std::string addressOutput = fmt::format("Block starting at 0x{:X}:\n{{}}", currentBlock.first);
			for (size_t i = 0; i < MiMa::MiMaMemory::MemoryBlock::MEMORY_BLOCK_SIZE; ++i) {
				std::string addressContent = fmt::format(addressFormat, (*(currentBlock.second))[i]);
				addressOutput = fmt::format(addressOutput, addressContent);
			}

			treeOutput = fmt::format(treeOutput, addressOutput);

			++it;
		}

		return fmt::format_to(ctx.out(), treeOutput, "");
	}
};
