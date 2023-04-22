#include "decl/reference.hpp"
#include "decl/class.hpp"
#include "decl/execution/stack.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"

#include <class_file/constant.hpp>

[[nodiscard]] inline optional<reference> try_get_field_resolved(
	instance_field_index resolved_field_index
) {
	reference ref = stack.pop_back<reference>();

	/* Otherwise, if objectref is null, the getfield instruction throws a
	   NullPointerException. */
	if(ref.is_null()) {
		return try_create_null_pointer_exception().get();
	}

	ref->view(
		resolved_field_index,
		[&](auto& field_value) {
			stack.emplace_back(field_value);
		}
	);

	return {};
}

[[nodiscard]] inline optional<reference> try_get_field_resolved(
	field& resolved_field
) {
	instance_field_index index =
		resolved_field._class().instance_fields()
		.find_index_of(resolved_field);

	return try_get_field_resolved(index);
}

[[nodiscard]] inline optional<reference> try_get_field(
	_class& d, class_file::constant::field_ref_index ref_index
) {
	/* The referenced field is resolved (§5.4.3.2). */
	expected<field&, reference> possible_resolved_field
		= d.try_get_resolved_field(
			ref_index,
			[&](field& f) -> optional<reference> {
				/* Otherwise, if the resolved field is a static field, getfield
				   throws an IncompatibleClassChangeError. */
				if(f.is_static()) {
					return try_create_null_pointer_exception().get();
				}
				return {};
			}
		);

	if(possible_resolved_field.is_unexpected()) {
		return possible_resolved_field.move_unexpected();
	}

	field& resolved_field = possible_resolved_field.get_expected();
	return try_get_field_resolved(resolved_field);
}