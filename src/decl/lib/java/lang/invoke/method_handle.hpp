#pragma once

#include "decl/class.hpp"
#include "decl/primitives.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"

#include <optional.hpp>
#include <overloaded.hpp>

namespace j {

struct method_handle : object {
	using object::object;

	static inline optional<::c&> c;
	static inline instance_method_index invoke_exact_ptr_index;
	static inline instance_method_index invoke_ptr_index;
	static inline layout::position is_varargs_field_position;
	static inline layout::position method_type_field_position;

	[[nodiscard]] inline optional<reference>
	try_invoke_exact() {
		method& m = object::c().instance_methods()
			[j::method_handle::invoke_exact_ptr_index];

		void* ptr0 = m.native_function();
		using f = optional<reference>(*)(
			j::method_handle& mh
		);
		return ((f)ptr0)(*this);
	}

	[[nodiscard]] inline optional<reference>
	try_invoke(
		j::method_type& t0_mt
	) {
		method& m = object::c().instance_methods()
			[j::method_handle::invoke_ptr_index];

		void* ptr0 = m.native_function();
		using f = optional<reference>(*)(
			j::method_type& t0_mt,
			j::method_handle& mh
		);
		return ((f)ptr0)(t0_mt, *this);
	}

	j::method_type& method_type() {
		object& obj = get<reference>(
			j::method_handle::method_type_field_position
		).object();
		return (j::method_type&) obj;
	}

	[[nodiscard]] inline optional<reference>
	try_invoke_with_arguments(object& args_array);

	bool is_varargs() {
		return get<bool>(is_varargs_field_position);
	}

};

}

#include "./method_handle.inc/check.hpp"
#include "./method_handle.inc/convert.hpp"
#include "./method_handle.inc/invoke.hpp"