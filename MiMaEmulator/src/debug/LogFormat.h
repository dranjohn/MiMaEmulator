#pragma once

//std library
#include <string>

//internal utility
#include "util/Tree.h"


namespace MiMa {
	std::string formatHierarchy(const Tree<std::string>& hierarchy);
}
