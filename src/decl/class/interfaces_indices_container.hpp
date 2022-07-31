#pragma once

#include "alloc.hpp"
#include <core/limited_list.hpp>

#include <class_file/constant.hpp>

struct interfaces_indices_container : ::limited_list<
	class_file::constant::interface_index, uint16, default_allocator
> {
	using base_type = ::limited_list<
		class_file::constant::interface_index, uint16, default_allocator
	>;
	using base_type::base_type;

};