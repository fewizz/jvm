#pragma once

#include "./find_by_name_and_descriptor_extension.hpp"
#include "./method_index.hpp"
#include "method.hpp"

#include <list.hpp>

#include <posix/memory.hpp>

struct instance_methods : posix::memory_for_range_of<method*> {
	using base_type = posix::memory_for_range_of<method*>;
	
	instance_methods(base_type&& methods) : base_type{ move(methods) } {}
};