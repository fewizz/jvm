#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"
#include "decl/classes.hpp"

#include <optional.hpp>

static optional<instance_method&> wrong_method_type_exception_constructor;

inline expected<reference, reference> try_create_wrong_method_type_exception() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/invoke/WrongMethodTypeException" }
	);
	instance_method& constructor
		= c.declared_instance_methods().find(
			c_string{ u8"<init>" }, c_string{ u8"()V" }
		);

	return try_create_object(constructor);
}