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
struct object_of<jl::i::method_handle> : object_of<jl::object> {
	using object_of<jl::object>::object_of;
};

namespace mh {

	[[nodiscard]] inline optional<reference>
	try_invoke_exact(
		object_of<jl::i::method_handle>& mh, nuint args_beginning
	) {
		method& m = mh.c().instance_methods()
			[jl::i::method_handle::invoke_exact_ptr_index];

		void* ptr0 = m.native_function();
		using f = optional<reference>(*)(
			reference mh_ref, nuint args_beginning
		);
		return ((f)ptr0)(mh, args_beginning);
	}

	[[nodiscard]] inline optional<reference>
	try_invoke(
		object_of<jl::i::method_handle>& mh,
		object_of<jl::i::method_type>& new_mt,
		nuint args_beginning
	) {
		method& m = mh.c().instance_methods()
			[jl::i::method_handle::invoke_exact_ptr_index];

		void* ptr0 = m.native_function();
		using f = optional<reference>(*)(
			reference mh_ref, reference new_mt, nuint args_beginning
		);
		return ((f)ptr0)(mh, new_mt, args_beginning);
	}

}

#include "./method_handle.inc/convert.hpp"
#include "./method_handle.inc/invoke.hpp"