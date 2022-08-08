#pragma once

#include "alloc.hpp"
#include <memory_list.hpp>

#include <class_file/constant.hpp>

struct interfaces_indices : ::memory_list<
	class_file::constant::interface_index, uint16
> {
	using base_type = ::memory_list<
		class_file::constant::interface_index, uint16
	>;
	using base_type::base_type;

};