#pragma once

#include "class.hpp"
#include <optional.hpp>

inline optional<_class&> method_type_class;
inline layout::position
	method_type_return_type_instance_field_position,
	method_type_parameter_types_instance_field_position,
	method_type_descriptor_instance_field_position;

static span<const char> method_type_descriptor(object& mt);

template<range_of<_class&> ParamClasses>
[[nodiscard]] inline expected<reference, reference> try_create_method_type(
	_class& ret_class,
	ParamClasses&& params_classes
);