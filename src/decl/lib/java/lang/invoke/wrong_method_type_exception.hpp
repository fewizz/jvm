#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"

#include <optional.hpp>

static optional<instance_method&> wrong_method_type_exception_constructor;

inline expected<reference, reference> try_create_wrong_method_type_exception() {
	return try_create_object(
		wrong_method_type_exception_constructor.get()
	);
}