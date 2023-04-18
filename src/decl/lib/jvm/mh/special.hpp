#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"

static optional<_class&> mh_special_class;
inline optional<method&> mh_special_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_special_mh(
	reference mt, _class& c, declared_instance_method_index index
) {
	return try_create_object(
		mh_special_constructor.get(),
		move(mt), c.instance(), uint16{ index }
	);
}

[[nodiscard]] inline expected<reference, reference>
try_create_special_mh(reference mt, method& m) {
	declared_instance_method_index index = m._class()
		.declared_instance_methods().find_index_of(m);

	return try_create_special_mh(move(mt), m._class(), index);
}