#pragma once

#include "field.hpp"
#include "class.hpp"
#include "reference.hpp"

[[nodiscard]] inline optional<field&> try_resolve_field0(
	_class& c,
	class_file::constant::utf8 name,
	class_file::constant::utf8 desc
) {
	/* When resolving a field reference, field resolution first attempts to look
	   up the referenced field in C and its superclasses: */

	/* If C declares a field with the name and descriptor specified by the field
	   reference, field lookup succeeds. The declared field is the result of the
	   field lookup. */
	optional<field&> f = c.declared_fields().try_find(name, desc);
	if(f.has_value()) {
		return f.get();
	}

	/* Otherwise, field lookup is applied recursively to the direct
	   superinterfaces of the specified class or interface C. */
	for(_class& i : c.declared_interfaces()) {
		optional<field&> possible_f =
			try_resolve_field0(i, name, desc);
		if(possible_f.has_value()) {
			return possible_f;
		}
	}

	/* Otherwise, if C has a superclass S, field lookup is applied recursively
	   to S. */
	if(c.has_super()) {
		optional<field&> possible_f =
			try_resolve_field0(c.super(), name, desc);
		if(possible_f.has_value()) {
			return possible_f;
		}
	}

	/* Otherwise, field lookup fails. */
	return {};
}

[[nodiscard]] inline expected<field&, reference> try_resolve_field(
	_class& c,
	class_file::constant::utf8 name,
	class_file::constant::utf8 desc
) {
	optional<field&> f = try_resolve_field0(c, name, desc);
	/* If field lookup failed, field resolution throws a NoSuchFieldError. */
	if(f.has_no_value()) {
		posix::abort(); // TODO throw
	}

	/* Otherwise, field lookup succeeded. Access control is applied for the
	   access from D to the field which is the result of field lookup
	   (§5.4.4). */
	// TODO access control

	return f.get();
}

#include "resolve_class.hpp"

/* To resolve an unresolved symbolic reference from D to a field in a class or
   interface C, the symbolic reference to C given by the field reference must
   first be resolved (§5.4.3.1). */
[[nodiscard]] inline expected<field&, reference> try_resolve_field(
	_class& d, class_file::constant::field_ref ref
) {
	class_file::constant::_class c0 = d.class_constant(ref.class_index);
	class_file::constant::utf8 class_name = d.utf8_constant(c0.name_index);
	expected<_class&, reference> possible_c = try_resolve_class(d, class_name);
	if(possible_c.is_unexpected()) {
		return move(possible_c.get_unexpected());
	}

	_class& c = possible_c.get_expected();

	class_file::constant::name_and_type nat
		= d.name_and_type_constant(ref.name_and_type_index);
	class_file::constant::utf8 name = d.utf8_constant(nat.name_index);
	class_file::constant::utf8 desc = d.utf8_constant(nat.descriptor_index);

	return try_resolve_field(c, name, desc);
}

[[nodiscard]] inline expected<field&, reference> try_resolve_field(
	_class& d, class_file::constant::field_ref_index ref_index
) {
	class_file::constant::field_ref ref = d.field_ref_constant(ref_index);
	return try_resolve_field(d, ref);
}