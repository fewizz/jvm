#pragma once

#include "../alloc.hpp"
#include <core/limited_list.hpp>

static inline limited_list<
	limited_list<uint8, uint32, default_allocator>,
	uint32,
	default_allocator
> strings{ 65536 };

