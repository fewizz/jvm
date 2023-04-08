#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"

static optional<_class&> mh_virtual_class;
inline optional<method&> mh_virtual_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_virtual_mh(
	reference mt, _class& c, instance_method_index index
) {
	return try_create_object(
		mh_virtual_constructor.get(),
		move(mt), c.instance(), uint16{ index }
	);
}

[[nodiscard]] inline expected<reference, reference>
try_create_virtual_mh(reference mt, method& m) {
	instance_method_index index = m._class()
		.instance_methods().find_index_of(m);

	return try_create_virtual_mh(move(mt), m._class(), index);
}