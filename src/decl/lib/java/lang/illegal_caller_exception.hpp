#pragma once

#include "reference.hpp"
#include "classes.hpp"

inline expected<reference, reference> try_create_illegal_caller_exception() {
	_class& c = classes.load_class_by_bootstrap_class_loader(
		c_string{"java/lang/IllegalCallerException"}
	);

	method& constructor = c.declared_instance_methods().find(
		c_string{"<init>"},
		c_string{"()V"}
	);

	return try_create_object(constructor);
}