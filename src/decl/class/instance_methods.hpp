#pragma once

#include "./find_by_name_and_descriptor_mixin.hpp"
#include "./method_index.hpp"
#include "method.hpp"

#include <memory_list.hpp>

struct instance_methods :
	memory_list<method&, instance_method_index>,
	find_by_name_and_descriptor_mixin<instance_methods>
{
	using base_type = memory_list<method&, instance_method_index>;
	using base_type::base_type;
};