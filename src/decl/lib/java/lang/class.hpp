#pragma once

#include "class/instance_field_position.hpp"

#include <optional.hpp>

struct _class;
struct object;

static optional<_class&> class_class{};
inline instance_field_position class_ptr_field_position;

static inline _class& class_from_class_instance(object& class_instance);