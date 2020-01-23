#pragma once

//internal utility
#include "MinType.h"


template <size_t index, size_t bitSize>
struct BitField {
private:
	typename MinType<index + bitSize>::type : index;
public:
	typename MinType<index + bitSize>::type value : bitSize;
};
