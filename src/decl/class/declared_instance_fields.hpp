#pragma once

#include "./field_index.hpp"
#include "field.hpp"

#include <posix/memory.hpp>

struct declared_instance_fields : posix::memory_for_range_of<field*> {
	using base_type = posix::memory_for_range_of<field*>;
	
	declared_instance_fields(base_type&& fields) :
		base_type{ move(fields) }
	{}
};