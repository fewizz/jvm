#pragma once

#include <posix/memory.hpp>

struct _class;

struct declared_interfaces : posix::memory_for_range_of<_class*> {
	using base_type = posix::memory_for_range_of<_class*>;

	declared_interfaces(posix::memory_for_range_of<_class*> s) :
		base_type{ move(s) }
	{}
};