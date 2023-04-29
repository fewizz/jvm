#pragma once

#include "class/layout.hpp"

#include <optional.hpp>

struct _class;
struct object;

static optional<c&> class_class{};
inline layout::position class_ptr_field_position;

static inline c& class_from_class_instance(object& class_instance);