#pragma once

#include "decl/method.hpp"
#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"

namespace jvm {

struct invoke_adapter : j::method_handle {
	static inline optional<::c&> c;
	static inline layout::position original_field_position;
	static inline optional<instance_method&> constructor;
		
	using j::method_handle::method_handle;

	j::method_handle& original() {
		reference& ref = get<reference>(
			original_field_position
		);
		return (j::method_handle&) ref.object();
	}
};

}