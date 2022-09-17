#pragma once

#include "field.hpp"
#include "./field_index.hpp"

#include <list.hpp>

#include <posix/memory.hpp>

struct declared_fields : list<posix::memory_for_range_of<field>> {
	using base_type = list<posix::memory_for_range_of<field>>;
	using base_type::base_type;
};