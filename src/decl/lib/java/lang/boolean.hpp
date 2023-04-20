#pragma once

#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/object.hpp"

#include <optional.hpp>

static optional<_class&> java_lang_boolean_class;
static optional<method&> java_lang_boolean_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_wrapper(same_as<bool> auto value) {
	return try_create_object(
		java_lang_boolean_constructor.get(),
		value
	);
}