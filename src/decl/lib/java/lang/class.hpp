#pragma once

#include "class/field_index.hpp"

#include <optional.hpp>

struct _class;
struct object;

static optional<_class&> class_class{};
inline instance_field_index class_ptr_field_index;

static inline _class& class_from_class_instance(object& class_instance);