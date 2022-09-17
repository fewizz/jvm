#pragma once

#include <list.hpp>

#include <posix/memory.hpp>

struct _class;

struct declared_interfaces : list<posix::memory_for_range_of<_class*>> {
	using base_type = list<posix::memory_for_range_of<_class*>>;
	using base_type::base_type;
};