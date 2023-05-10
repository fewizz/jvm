#pragma once

#include "reference.hpp"
#include "classes.hpp"

inline expected<reference, reference> try_create_illegal_caller_exception() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/IllegalCallerException" }
	);

	method& constructor = c.declared_instance_methods().find(
		c_string{ u8"<init>" },
		c_string{ u8"()V" }
	);

	return try_create_object(constructor);
}