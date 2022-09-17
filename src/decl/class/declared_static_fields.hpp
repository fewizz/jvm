#pragma once

#include "./find_by_name_and_descriptor_extension.hpp"
#include "./field_index.hpp"
#include "field.hpp"

#include <list.hpp>
#include <posix/memory.hpp>

struct declared_static_fields :
	list<posix::memory_for_range_of<field*>>,
	find_by_name_and_descriptor_extension<declared_static_fields, true>
{
private:
	using base_type = list<posix::memory_for_range_of<field*>>;
public:

	using base_type::base_type;
};