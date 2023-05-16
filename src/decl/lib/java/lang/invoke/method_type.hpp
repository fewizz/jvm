#pragma once

#include "class.hpp"
#include "object.hpp"
#include "array.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/object.hpp"

#include <optional.hpp>
#include <generator_view.hpp>

#include <unicode/utf8.hpp>

namespace jl::i {

	struct method_type {
		static inline optional<::c&> c;
		static inline layout::position
			return_type_instance_field_position,
			parameter_types_instance_field_position,
			descriptor_instance_field_position;
	};

}

template<range_of<c&> ParamClasses>
[[nodiscard]] inline expected<reference, reference> try_create_method_type(
	::c& ret_class,
	ParamClasses&& params_classes
);

inline expected<reference, reference>
try_create_method_type_generic(nuint count) {
	generator_view param_types {
		[]() -> c& { return object_class.get(); },
		count
	};
	return try_create_method_type(object_class.get(), param_types);
}

template<>
struct object_of<jl::i::method_type> : object_of<jl::object> {
	using object_of<jl::object>::object_of;

	span<utf8::unit> descriptor() {
		reference& utf8_desc = get<reference>(
			jl::i::method_type::descriptor_instance_field_position
		);
		return array_as_span<utf8::unit>(utf8_desc);
	}

	::c& return_type() {
		reference& return_type = get<reference>(
			jl::i::method_type::return_type_instance_field_position
		);
		return class_from_class_instance(return_type);
	}

	auto parameter_types_view() {
		reference& parameter_types_array = get<reference>(
			jl::i::method_type::parameter_types_instance_field_position
		);

		span<reference> parameter_types
			= array_as_span<reference>(parameter_types_array);

		return move(parameter_types).transform_view([](reference& ref) -> ::c& {
			return class_from_class_instance(ref);
		});
	}

};