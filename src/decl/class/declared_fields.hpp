#pragma once

#include "field.hpp"
#include "./field_index.hpp"

#include <list.hpp>

#include <posix/memory.hpp>

struct declared_fields : posix::memory_for_range_of<field> {
	using base_type = posix::memory_for_range_of<field>;

	declared_fields(posix::memory_for_range_of<field> s) :
		base_type{ move(s) }
	{}
};