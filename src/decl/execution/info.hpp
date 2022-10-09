#pragma once

#include "print.hpp"

#include <integer.hpp>

#ifndef PRINT_EXECUTION_INFO
#define PRINT_EXECUTION_INFO false
#endif

inline static constexpr bool info = PRINT_EXECUTION_INFO;
inline static nuint tab = 0;

inline void tabs() {
	if(tab == 0) return;

	nuint size = tab * 2;
	char chrs[tab * 2];

	for(nuint i = 0; i < size; ++i) { chrs[i] = ' '; }

	print(span{ chrs, size });
}