#pragma once

#include <class_file/constant.hpp>

#include <posix/memory.hpp>

struct bootstrap_method {
	class_file::constant::method_handle_index method_handle_index;
	posix::memory<class_file::constant::index> arguments_indices;

	bootstrap_method(
		class_file::constant::method_handle_index
			method_handle_index,
		posix::memory<class_file::constant::index>
			arguments_indices
	) :
		method_handle_index{ method_handle_index },
		arguments_indices{ move(arguments_indices) }
	{}

	bootstrap_method(bootstrap_method&&) = default;
	bootstrap_method& operator = (bootstrap_method&&) = default;
};

struct bootstrap_methods : initialised<posix::memory<bootstrap_method>> {
	using base_type = initialised<posix::memory<bootstrap_method>>;
	using base_type::base_type;

	bootstrap_methods(bootstrap_methods&&) = default;
	bootstrap_methods& operator = (bootstrap_methods&&) = default;

};