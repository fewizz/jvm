#pragma once

#include "print.hpp"

#include <integer.hpp>

inline static constexpr bool info = true;
inline static nuint tab = 0;

inline void tabs() {
	for(nuint i = 0; i < tab; ++i) {
		print("  ");
	}
}