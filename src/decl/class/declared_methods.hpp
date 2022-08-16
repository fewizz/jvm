#pragma once

#include "./find_by_name_and_descriptor_mixin.hpp"
#include "./method_index.hpp"
#include "method.hpp"

#include <memory_list.hpp>

struct declared_methods :
	memory_list<method, declared_method_index>,
	find_by_name_and_descriptor_mixin<::declared_methods>
{
private:
	using base_type = memory_list<method, declared_method_index>;
public:
	using base_type::base_type;
};