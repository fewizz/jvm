#pragma once

#include "./method_index.hpp"
#include "./find_by_name_and_descriptor_extension.hpp"
#include "method.hpp"

#include <list.hpp>

#include <posix/memory.hpp>

struct declared_instance_methods :
	list<posix::memory_for_range_of<method*>>,
	find_by_name_and_descriptor_extension<declared_instance_methods, true>
{
	using base_type = list<posix::memory_for_range_of<method*>>;
	using base_type::base_type;
};