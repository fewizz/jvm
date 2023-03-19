#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"

#include <optional.hpp>

static optional<method&> wrong_method_type_exception_constructor;

inline reference create_wrong_method_type_exception() {
	reference ref = create_object(
		wrong_method_type_exception_constructor->_class()
	);
	
	execute(wrong_method_type_exception_constructor.get(), ref);

	return ref;
}