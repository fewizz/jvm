#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"

static optional<c&> mh_special_class;
inline optional<method&> mh_special_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_special_mh(
	reference mt, c& c, declared_instance_method_index index
) {
	return try_create_object(
		mh_special_constructor.get(),
		move(mt), reference{ c.object() }, uint16{ index }
	);
}

[[nodiscard]] inline expected<reference, reference>
try_create_special_mh(reference mt, instance_method& m) {
	declared_instance_method_index index = m.c()
		.declared_instance_methods().find_index_of(m);

	return try_create_special_mh(move(mt), m.c(), index);
}