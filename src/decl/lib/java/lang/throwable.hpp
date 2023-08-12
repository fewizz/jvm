#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"

static optional<c&> throwable_class;
inline layout::position throwable_stack_trace_field_position;

namespace j {

	struct throwable : object {
		using object::object;

		inline void init_cause(reference cause);
	};

}