#pragma once

#include <class_file/constant.hpp>

#include <posix/memory.hpp>

struct bootstrap_method_arguments_indices :
	posix::memory<class_file::constant::index>
{
	using base_type = posix::memory<class_file::constant::index>;
	using base_type::base_type;

	bootstrap_method_arguments_indices(
		posix::memory<class_file::constant::index> mem
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

	bootstrap_method(bootstrap_method&&) = default;
	bootstrap_method& operator = (bootstrap_method&&) = default;

	~bootstrap_method() {
		for(auto& s : arguments_indices) {
			s.destruct();
		}
	}
};

struct bootstrap_methods :
	posix::memory<bootstrap_method>
{
	using base_type = posix::memory<bootstrap_method>;
	using base_type::base_type;

	bootstrap_methods(
		posix::memory<bootstrap_method> mem
	) :
		base_type{ move(mem) }
	{}

	bootstrap_methods(bootstrap_methods&&) = default;
	bootstrap_methods& operator = (bootstrap_methods&&) = default;

	~bootstrap_methods() {
		for(auto& s : *this) {
			s.destruct();
		}
	}

};