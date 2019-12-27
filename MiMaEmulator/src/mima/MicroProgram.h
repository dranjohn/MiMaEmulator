#pragma once

#include <cstdint>
#include <istream>
#include <memory>
#include <string>
#include <map>
#include <functional>

#include "util/CharStream.h"

namespace MiMa {
	class MicroProgram {
	private:
		//program memory
		std::shared_ptr<uint32_t[]> memory;
	public:
		MicroProgram(std::shared_ptr<uint32_t[]>& memory) : memory(memory) {}

		inline std::shared_ptr<uint32_t[]> getMemory() { return memory; }
	};
}
