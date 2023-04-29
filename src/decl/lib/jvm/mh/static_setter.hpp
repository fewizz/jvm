#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"

static optional<c&> mh_static_setter_class;
static optional<method&> mh_static_setter_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_static_setter_mh(
	reference mt,
	c& c,
	declared_static_field_index field_index
) {
	return try_create_object(
		mh_static_setter_constructor.get(),
		move(mt), reference{ c.object() }, uint16{ field_index }
	);
}

[[nodiscard]] inline expected<reference, reference>
try_create_static_setter_mh(
	reference mt, field& f
) {
	declared_static_field_index index = f.c()
		.declared_static_fields().find_index_of(f);
	return try_create_static_setter_mh(move(mt), f.c(), index);
}