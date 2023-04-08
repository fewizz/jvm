#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"

static optional<_class&> mh_constructor_class;
inline optional<method&> mh_constructor_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_constructor_mh(
	reference mt, _class& c, declared_instance_method_index index
) {
	return try_create_object(
		mh_constructor_constructor.get(),
		move(mt), c.instance(), uint16{ index }
	);
}

[[nodiscard]] inline expected<reference, reference>
try_create_constructor_mh(reference mt, method& m) {
	declared_instance_method_index index = m._class()
		.declared_instance_methods().find_index_of(m);

	return try_create_constructor_mh(move(mt), m._class(), index);
}