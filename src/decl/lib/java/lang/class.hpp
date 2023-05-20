#pragma once

#include "decl/class/layout.hpp"
#include "decl/object.hpp"

#include <optional.hpp>

namespace jl {
	struct c{};
}

static optional<c&> class_class{};
inline layout::position class_ptr_field_position;

static inline c& class_from_class_instance(o<jl::object>& class_instance);

template<>
struct o<jl::c> : o<jl::object> {
	using o<jl::object>::o;

	::c& c() {
		return class_from_class_instance(*this);
	}

};