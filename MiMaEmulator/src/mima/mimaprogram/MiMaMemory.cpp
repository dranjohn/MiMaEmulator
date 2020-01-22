#include "MiMaMemory.h"

namespace MiMa {
	const std::function<std::shared_ptr<MiMaMemory::MemoryBlock>(void)> MiMaMemory::emptyMemoryBlockConstructor = std::make_shared<MiMaMemory::MemoryBlock>;

	MemoryCell& MiMaMemory::operator[](const size_t& index) {
		size_t offset = index % 256;
		size_t blockIndex = (index - offset) / 256;
		
		return (*(memory.find(blockIndex)))[offset];
	}
}
