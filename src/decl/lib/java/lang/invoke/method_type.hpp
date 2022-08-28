#pragma once

#include "class.hpp"
#include <optional.hpp>

inline optional<_class&> method_type_class;
inline instance_field_index method_type_return_type_instance_field_index;
inline instance_field_index method_type_parameter_types_instance_field_index;
inline instance_field_index method_type_descriptor_instance_field_index;

static span<const char> method_type_descriptor(object& mt);