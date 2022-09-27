#pragma once

#include "print.hpp"

#include <integer.hpp>

#ifndef PRINT_EXECUTION_INFO
#define PRINT_EXECUTION_INFO false
#endif

inline static constexpr bool info = PRINT_EXECUTION_INFO;
inline static nuint tab = 0;

inline void tabs() {
	for(nuint i = 0; i < tab; ++i) {
		print("  ");
	}
}