#pragma once

#include "../alloc.hpp"
#include <core/fixed_vector.hpp>

static inline fixed_vector<
	fixed_vector<uint8, uint32, default_allocator>,
	uint32,
	default_allocator
> strings{ 65536 };

