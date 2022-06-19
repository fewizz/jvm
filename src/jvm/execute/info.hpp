#pragma once

#include <core/integer.hpp>
#include <stdio.h>

inline static constexpr bool info = true;
inline static nuint tab = 0;

inline void tabs() {
	for(nuint i = 0; i < tab; ++i) fputc('\t', stderr);
}