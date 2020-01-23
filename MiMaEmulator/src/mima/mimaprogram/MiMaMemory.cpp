#include "mimapch.h"
#include "MiMaMemory.h"

namespace MiMa {
	const std::function<std::shared_ptr<MiMaMemory::MemoryBlock>(void)> MiMaMemory::emptyMemoryBlockConstructor = std::make_shared<MiMaMemory::MemoryBlock>;

	MemoryCell& MiMaMemory::operator[](const size_t& index) {
		size_t offset = index % MiMaMemory::MemoryBlock::MEMORY_BLOCK_SIZE;
		size_t blockIndex = (index - offset) / MiMaMemory::MemoryBlock::MEMORY_BLOCK_SIZE;
		
		return (*(memory.find(blockIndex)))[offset];
	}
}
