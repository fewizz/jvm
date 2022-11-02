#pragma once

#include "./find_by_name_and_descriptor_extension.hpp"
#include "./field_index.hpp"
#include "field.hpp"

#include <list.hpp>

#include <posix/memory.hpp>

struct instance_fields : posix::memory_for_range_of<field*> {
	using base_type = posix::memory_for_range_of<field*>;

	instance_fields(posix::memory_for_range_of<field*> storage) :
		base_type{ move(storage) }
	{}
};