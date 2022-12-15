#pragma once

#include <class_file/constant.hpp>

#include <posix/memory.hpp>

struct bootstrap_method_arguments_indices :
	posix::memory_for_range_of<class_file::constant::index>
{
	using base_type = posix::memory_for_range_of<class_file::constant::index>;
	using base_type::base_type;

	bootstrap_method_arguments_indices(
		posix::memory_for_range_of<class_file::constant::index> mem
	) :
		base_type{ move(mem) }
	{}
};

struct bootstrap_method {
	class_file::constant::method_handle_index method_handle_index;
	bootstrap_method_arguments_indices        arguments_indices;

	bootstrap_method(
		class_file::constant::method_handle_index method_handle_index,
		bootstrap_method_arguments_indices        arguments_indices
	) :
		method_handle_index{ method_handle_index },
		arguments_indices{ move(arguments_indices) }
	{}
};

struct bootstrap_methods :
	posix::memory_for_range_of<bootstrap_method>
{
	using base_type = posix::memory_for_range_of<bootstrap_method>;
	using base_type::base_type;

	bootstrap_methods(posix::memory_for_range_of<bootstrap_method> mem) :
		base_type{ move(mem) }
	{}
};