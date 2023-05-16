#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"

static optional<c&> mh_virtual_class;
static optional<instance_method&> mh_virtual_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_virtual_mh(
	object_of<jl::i::method_type>& mt,
	method& resolved_method
) {
	declared_instance_method_index resolved_method_index =
		resolved_method.c()
		.declared_instance_methods().find_index_of(resolved_method);

	return try_create_object(
		mh_virtual_constructor.get(),
		mt,
		resolved_method.c().object(),
		uint16{ resolved_method_index }
	);
}