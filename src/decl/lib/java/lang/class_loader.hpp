#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"

namespace j {

struct c_loader : object {
	using object::object;

	inline static instance_method_index load_class_method_index;
};

}