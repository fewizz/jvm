#pragma once

#include "decl/class/layout.hpp"

#include <optional.hpp>

struct _class;
struct method;

static optional<_class&> java_lang_byte_class;
static optional<method&> java_lang_byte_constructor;
static layout::position java_lang_byte_value_field_position;