#pragma once

#include "./find_by_name_and_descriptor_extension.hpp"
#include "./method_index.hpp"
#include "method.hpp"

#include <posix/memory.hpp>

struct declared_methods : posix::memory_for_range_of<method> {
private:
	using base_type = posix::memory_for_range_of<method>;
public:

	declared_methods(posix::memory_for_range_of<method> s) :
		base_type{ move(s) }
	{}

};