#pragma once

//std library
#include <cstddef>
#include <cstdint>
#include <type_traits>


template <size_t bitSize>
struct MinType {
	typedef
		typename std::conditional<bitSize == 0, void,
		typename std::conditional<bitSize <= 8, std::uint8_t,
		typename std::conditional<bitSize <= 16, std::uint16_t,
		typename std::conditional<bitSize <= 32, std::uint32_t,
		typename std::conditional<bitSize <= 64, std::uint64_t,
		void>::type>::type>::type>::type>::type type;
};
