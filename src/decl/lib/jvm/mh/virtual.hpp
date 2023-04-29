#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"

static optional<c&> mh_virtual_class;
static optional<method&> mh_virtual_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_virtual_mh(reference mt, method& resolved_method) {
	declared_instance_method_index resolved_method_index =
		resolved_method.c()
		.declared_instance_methods().find_index_of(resolved_method);

	return try_create_object(
		mh_virtual_constructor.get(),
		move(mt),
		reference{ resolved_method.c().object() },
		uint16{ resolved_method_index }
	);
}