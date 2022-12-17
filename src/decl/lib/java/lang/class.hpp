#pragma once

#include "class/layout.hpp"

#include <optional.hpp>

struct _class;
struct object;

static optional<_class&> class_class{};
inline layout::position class_ptr_field_position;

static inline _class& class_from_class_instance(object& class_instance);