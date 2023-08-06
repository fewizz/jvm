#pragma once

#include "decl/class/layout.hpp"

#include <optional.hpp>

struct _class;
struct instance_method;

static optional<c&> java_lang_integer_class;
static optional<instance_method&> java_lang_integer_constructor;
static layout::position java_lang_integer_value_field_position;