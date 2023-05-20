#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"

namespace jl {

	struct c_loader{};

}

inline instance_method_index class_loader_load_class_method_index;

template<>
struct o<jl::c_loader> : o<jl::object> {
	using o<jl::object>::o;
};