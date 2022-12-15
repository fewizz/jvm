#pragma once

#include "./find_by_name_and_descriptor_extension.hpp"
#include "./method_index.hpp"

#include <posix/memory.hpp>

struct method;

struct declared_static_methods : posix::memory_for_range_of<method*> {
private:
	using base_type = posix::memory_for_range_of<method*>;
public:

	declared_static_methods(base_type&& methods) :
		base_type{ move(methods) }
	{}
};