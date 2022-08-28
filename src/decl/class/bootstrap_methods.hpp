#pragma once

#include "alloc.hpp"

#include <class_file/constant.hpp>

#include <memory_list.hpp>

struct bootstrap_method_arguments_indices :
	memory_list<class_file::constant::index, uint16>
{
	using base_type = memory_list<class_file::constant::index, uint16>;
	using base_type::base_type;
};

struct bootstrap_method {
	class_file::constant::method_handle_index method_handle_index;
	bootstrap_method_arguments_indices        arguments_indices;
};

struct bootstrap_methods :
	memory_list<bootstrap_method, uint16>
{
	using base_type = memory_list<bootstrap_method, uint16>;
	using base_type::base_type;
};