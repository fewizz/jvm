#pragma once

#include <class_file/constant.hpp>

#include <list.hpp>

#include <posix/memory.hpp>

struct bootstrap_method_arguments_indices :
	list<posix::memory_for_range_of<class_file::constant::index>>
{
	using base_type = list<posix::memory_for_range_of<class_file::constant::index>>;
	using base_type::base_type;
};

struct bootstrap_method {
	class_file::constant::method_handle_index method_handle_index;
	bootstrap_method_arguments_indices        arguments_indices;
};

struct bootstrap_methods :
	list<posix::memory_for_range_of<bootstrap_method>>
{
	using base_type = list<posix::memory_for_range_of<bootstrap_method>>;
	using base_type::base_type;
};