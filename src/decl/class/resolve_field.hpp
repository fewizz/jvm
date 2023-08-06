#pragma once

#include "field.hpp"
#include "class.hpp"
#include "reference.hpp"
#include "decl/lib/java/lang/no_such_field_error.hpp"
#include "decl/class/access_control.hpp"

#include <ranges.hpp>

[[nodiscard]] inline optional<field&> try_resolve_field0(
	c& c,
	class_file::constant::utf8 name,
	class_file::constant::utf8 desc
) {
	/* When resolving a field reference, field resolution first attempts to look
	   up the referenced field in C and its superclasses: */

	/* If C declares a field with the name and descriptor specified by the field
	   reference, field lookup succeeds. The declared field is the result of the
	   field lookup. */
	optional<field&> f =
		find_by_name_and_descriptor_view {
			ranges{ c.declared_static_fields(), c.declared_instance_fields() }
			.concat_view()
		}
		.try_find(name, desc);

	if(f.has_value()) {
		return f.get();
	}

	/* Otherwise, field lookup is applied recursively to the direct
	   superinterfaces of the specified class or interface C. */
	for(::c& i : c.declared_interfaces()) {
		optional<field&> possible_f = try_resolve_field0(i, name, desc);
		if(possible_f.has_value()) {
			return possible_f;
		}
	}

	/* Otherwise, if C has a superclass S, field lookup is applied recursively
	   to S. */
	if(c.has_super()) {
		optional<field&> possible_f = try_resolve_field0(c.super(), name, desc);
		if(possible_f.has_value()) {
			return possible_f;
		}
	}

	/* Otherwise, field lookup fails. */
	return {};
}

[[nodiscard]] inline expected<field&, reference> try_resolve_field(
	c& d,
	c& c,
	class_file::constant::utf8 name,
	class_file::constant::utf8 desc
) {
	optional<field&> possible_f = try_resolve_field0(c, name, desc);
	/* If field lookup failed, field resolution throws a NoSuchFieldError. */
	if(possible_f.has_no_value()) {
		return unexpected{ try_create_no_such_field_error().get() };
	}

	field& f = possible_f.get();

	/* Otherwise, field lookup succeeded. Access control is applied for the
	   access from D to the field which is the result of field lookup
	   (§5.4.4). */
	optional<reference> possible_exception = access_control(d, f);
	if(possible_exception.has_value()) {
		return unexpected{ possible_exception.move() };
	}
	// TODO loading constraints

	return f;
}

#include "resolve_class.hpp"

/* To resolve an unresolved symbolic reference from D to a field in a class or
   interface C, the symbolic reference to C given by the field reference must
   first be resolved (§5.4.3.1). */
[[nodiscard]] inline expected<field&, reference> c::try_resolve_field(
	class_file::constant::field_ref ref
) {
	class_file::constant::_class c0 = (*this)[ref.class_constant_index];
	class_file::constant::utf8 class_name = (*this)[c0.name_constant_index];
	expected<c&, reference> possible_c
		= try_resolve_class(*this, class_name);
	if(possible_c.is_unexpected()) {
		return possible_c.move_unexpected();
	}

	c& c = possible_c.get_expected();

	class_file::constant::name_and_type nat
		= (*this)[ref.name_and_type_constant_index];
	class_file::constant::utf8 name = (*this)[nat.name_constant_index];
	class_file::constant::utf8 desc = (*this)[nat.descriptor_constant_index];

	return ::try_resolve_field(*this, c, name, desc);
}