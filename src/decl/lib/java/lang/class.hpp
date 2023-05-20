#pragma once

#include "decl/class/layout.hpp"
#include "decl/object.hpp"

#include <optional.hpp>

static optional<c&> class_class{};
inline layout::position class_ptr_field_position;

static inline c& class_from_class_instance(object& class_instance);

namespace j {

struct c : object {
	using object::object;

	::c& get_c() {
		return class_from_class_instance(*this);
	}

};

}