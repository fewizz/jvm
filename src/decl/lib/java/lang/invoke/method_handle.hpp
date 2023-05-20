#pragma once

#include "decl/class.hpp"
#include "decl/descriptor.hpp"
#include "decl/primitives.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"
#include "decl/lib/java/lang/object.hpp"

#include <optional.hpp>
#include <overloaded.hpp>

namespace jl::i {
	struct method_handle {
		static inline optional<::c&> c;
		static inline instance_method_index invoke_exact_ptr_index;
		static inline layout::position method_type_field_position;
	};
}

template<>
struct o<jl::i::method_handle> : o<jl::object> {
	using o<jl::object>::o;

	[[nodiscard]] inline optional<reference>
	try_invoke_exact(nuint args_beginning) {
		method& m = c().instance_methods()
			[jl::i::method_handle::invoke_exact_ptr_index];

		void* ptr0 = m.native_function();
		using f = optional<reference>(*)(
			reference mh_ref, nuint args_beginning
		);
		return ((f)ptr0)(*this, args_beginning);
	}

	[[nodiscard]] inline optional<reference>
	try_invoke(
		o<jl::i::method_type>& new_mt,
		nuint args_beginning
	) {
		method& m = c().instance_methods()
			[jl::i::method_handle::invoke_exact_ptr_index];

		void* ptr0 = m.native_function();
		using f = optional<reference>(*)(
			reference mh_ref, reference new_mt, nuint args_beginning
		);
		return ((f)ptr0)(*this, new_mt, args_beginning);
	}
};

#include "./method_handle.inc/convert.hpp"
#include "./method_handle.inc/invoke.hpp"