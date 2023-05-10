#pragma once

#include "decl/class.hpp"
#include "decl/descriptor.hpp"
#include "decl/primitives.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"
#include "decl/lib/java/lang/object.hpp"

#include <optional.hpp>
#include <overloaded.hpp>

static optional<c&> method_handle_class;

inline instance_method_index method_handle_invoke_exact_ptr_index;
inline layout::position method_handle_method_type_field_position;

namespace mh {

	[[nodiscard]] inline optional<reference>
	try_invoke_exact(reference mh_ref, nuint args_beginning) {
		method& m = mh_ref->c().instance_methods()
			[method_handle_invoke_exact_ptr_index];

		void* ptr0 = m.native_function();
		using f = optional<reference>(*)(
			reference mh_ref, nuint args_beginning
		);
		return ((f)ptr0)(move(mh_ref), args_beginning);
	}

	[[nodiscard]] inline optional<reference>
	try_invoke(
		reference mh_ref, reference new_mt, nuint args_beginning
	) {
		method& m = mh_ref->c().instance_methods()
			[method_handle_invoke_exact_ptr_index];

		void* ptr0 = m.native_function();
		using f = optional<reference>(*)(
			reference mh_ref, reference new_mt, nuint args_beginning
		);
		return ((f)ptr0)(move(mh_ref), move(new_mt), args_beginning);
	}

}

#include "./method_handle.inc/convert.hpp"
#include "./method_handle.inc/invoke.hpp"