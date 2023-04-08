#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"

static optional<_class&> mh_setter_class;
static optional<method&> mh_setter_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_setter_mh(
	reference mt,
	_class& c,
	instance_field_index field_index
) {
	return try_create_object(
		mh_setter_constructor.get(),
		move(mt), c.instance(), uint16{ field_index }
	);
}

[[nodiscard]] inline expected<reference, reference>
try_create_setter_mh(
	reference mt, field& f
) {
	instance_field_index index = f._class().instance_fields().find_index_of(f);
	return try_create_setter_mh(move(mt), f._class(), index);
}