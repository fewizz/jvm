#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"

static optional<c&> mh_static_class;
inline optional<instance_method&> mh_static_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_static_mh(
	object_of<jl::i::method_type>& mt,
	c& c,
	declared_static_method_index index
) {
	return try_create_object(
		mh_static_constructor.get(),
		mt, c.object(), uint16{ index }
	);
}

[[nodiscard]] inline expected<reference, reference>
try_create_static_mh(object_of<jl::i::method_type>& mt, method& m) {
	declared_static_method_index index = m.c()
		.declared_static_methods().find_index_of(m);

	return try_create_static_mh(mt, m.c(), index);
}