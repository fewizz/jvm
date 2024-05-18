#pragma once

#include "reference.hpp"
#include "classes.hpp"

inline expected<reference, reference> try_create_illegal_caller_exception() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/IllegalCallerException"sv
	);

	instance_method& constructor = c.declared_instance_methods().find(
		u8"<init>"sv,
		u8"()V"sv
	);

	return try_create_object(constructor);
}