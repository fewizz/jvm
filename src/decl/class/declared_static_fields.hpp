#pragma once

#include "./find_by_name_and_descriptor_extension.hpp"
#include "./field_index.hpp"
#include "field.hpp"

#include <list.hpp>
#include <posix/memory.hpp>

struct declared_static_fields : posix::memory_for_range_of<field*> {
private:
	using base_type = posix::memory_for_range_of<field*>;
public:

	declared_static_fields(base_type&& fields) :
		base_type{ move(fields) }
	{}
};