#pragma once

#include "method.hpp"

struct methods_container :
	limited_list<method, uint16, default_allocator>
{
	using base_type = limited_list<method, uint16, default_allocator>;
	using base_type::base_type;
};