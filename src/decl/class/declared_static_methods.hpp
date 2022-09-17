#pragma once

#include "./find_by_name_and_descriptor_extension.hpp"
#include "./method_index.hpp"

#include <list.hpp>

#include <posix/memory.hpp>

struct method;

struct declared_static_methods :
	list<posix::memory_for_range_of<method*>>,
	find_by_name_and_descriptor_extension<declared_static_methods, true>
{
private:
	using base_type = list<posix::memory_for_range_of<method*>>;
public:

	using base_type::base_type;
};