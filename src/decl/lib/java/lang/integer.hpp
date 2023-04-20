#pragma once

#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/object.hpp"

#include <optional.hpp>

static optional<_class&> java_lang_integer_class;
static optional<method&> java_lang_integer_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_wrapper(same_as<int32> auto value) {
	return try_create_object(java_lang_integer_constructor.get(), value);
}