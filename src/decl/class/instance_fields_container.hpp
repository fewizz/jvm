#pragma once

#include "field/instance.hpp"
#include "alloc.hpp"

#include <core/limited_list.hpp>

struct instance_fields_container :
	limited_list<instance_field, uint16, default_allocator>
{
	using base_type = limited_list<instance_field, uint16, default_allocator>;
	using base_type::base_type;
};