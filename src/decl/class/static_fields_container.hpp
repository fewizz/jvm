#pragma once

#include "field/static.hpp"

#include <core/limited_list.hpp>

struct static_fields_container :
	limited_list<static_field, uint16, default_allocator>
{
	using base_type = limited_list<static_field, uint16, default_allocator>;
	using base_type::base_type;
};