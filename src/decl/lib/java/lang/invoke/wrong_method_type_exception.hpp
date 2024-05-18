#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"
#include "decl/classes.hpp"

#include <optional.hpp>

static optional<instance_method&> wrong_method_type_exception_constructor;

inline expected<reference, reference> try_create_wrong_method_type_exception() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/invoke/WrongMethodTypeException"sv
	);
	instance_method& constructor
		= c.declared_instance_methods().find(
			u8"<init>"sv, u8"()V"sv
		);

	return try_create_object(constructor);
}