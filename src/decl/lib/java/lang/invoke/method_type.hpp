#pragma once

#include "class.hpp"
#include "object.hpp"
#include "array.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/object.hpp"

#include <optional.hpp>
#include <generator_view.hpp>

#include <unicode/utf8.hpp>

inline optional<::c&> method_type_class;
inline layout::position
	method_type_return_type_instance_field_position,
	method_type_parameter_types_instance_field_position,
	method_type_descriptor_instance_field_position;

static span<utf8::unit> method_type_descriptor(object& mt);

template<range_of<c&> ParamClasses>
[[nodiscard]] inline expected<reference, reference> try_create_method_type(
	::c& ret_class,
	ParamClasses&& params_classes
);

inline auto method_type_parameter_types_view(object& mt) {
	reference& parameter_types_array = mt.get<reference>(
		method_type_parameter_types_instance_field_position
	);

	span<reference> parameter_types
		= array_as_span<reference>(parameter_types_array);

	return move(parameter_types).transform_view([](reference& ref) -> c& {
		return class_from_class_instance(ref);
	});
}

inline c& method_type_return_type(object& mt) {
	reference& return_type = mt.get<reference>(
		method_type_return_type_instance_field_position
	);
	return class_from_class_instance(return_type);
}

inline expected<reference, reference>
try_create_method_type_generic(nuint count) {
	generator_view param_types {
		[]() -> c& { return object_class.get(); },
		count
	};
	return try_create_method_type(object_class.get(), param_types);
}