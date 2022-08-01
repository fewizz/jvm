#pragma once

#include "alloc.hpp"

#include <class_file/constant.hpp>
#include <class_file/attribute/bootstrap/method/argument_index.hpp>

#include <core/limited_list.hpp>

struct bootstrap_method_arguments_indices :
	limited_list<
		class_file::attribute::bootstrap::method::argument_index,
		uint16,
		default_allocator
	>
{
	using base_type = limited_list<
		class_file::attribute::bootstrap::method::argument_index,
		uint16,
		default_allocator
	>;
	using base_type::base_type;
};

struct bootstrap_method {
	class_file::constant::method_handle_index method_handle_index;
	bootstrap_method_arguments_indices arguments_indices;
};

struct bootstrap_method_pool :
	limited_list<bootstrap_method, uint16, default_allocator>
{
	using base_type = limited_list<bootstrap_method, uint16, default_allocator>;
	using base_type::base_type;
};