#include "MiMaMemory.h"

namespace MiMa {
	MiMaMemory::MiMaMemory() {
		memory.reset(new MemoryCell[DEFAULT_MEMORY_CAPACITY]);
	}
}
