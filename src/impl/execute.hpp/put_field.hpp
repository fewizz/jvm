#include "decl/reference.hpp"
#include "decl/class.hpp"
#include "decl/execution/stack.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/lib/java/lang/illegal_access_error.hpp"

#include <class_file/constant.hpp>

[[nodiscard]] inline optional<reference>
try_put_field_resolved(field& resolved_field) {
	reference ref = move(stack.get<reference>(
		stack.size() - resolved_field.stack_size - 1
	));

	/* Otherwise, if objectref is null, the putfield instruction throws a
	   NullPointerException. */
	if(ref.is_null()) {
		return try_create_null_pointer_exception().get();
	}

	instance_field_index index =
		resolved_field._class().instance_fields()
		.find_index_of(resolved_field);

	ref->view(
		index,
		[&]<typename FieldType>(FieldType& field_value) {
			/* If the value is of type int and the field descriptor type is
			   boolean, then the int value is narrowed by taking the bitwise AND
			   of value and 1, resulting in value'. The referenced field in
			   objectref is set to value'. */
			// put automagically?
			field_value = stack.pop_back<FieldType>();
		}
	);

	stack.pop_back<reference>(); // pop 'this'

	return {};
}

[[nodiscard]] inline optional<reference>
try_put_field(
	method& current_method,
	class_file::constant::field_ref_index ref_index
) {
	_class& d = current_method._class();

	expected<field&, reference> possible_resolved_field =
		d.try_get_resolved_field(
			ref_index,
			[&](field& f) -> optional<reference> {
				/* Otherwise, if the resolved field is a static field, putfield
				throws an IncompatibleClassChangeError. */
				if(f.is_static()) {
					return try_create_incompatible_class_change_error().get();
				}

				/* Otherwise, if the resolved field is final, it must be
				declared in the current class, and the instruction must occur in
				an instance initialization method of the current class.
				Otherwise, an IllegalAccessError is thrown. */
				if(
					f.is_final() && !(
						f._class().is(d) &&
						current_method.is_instance_initialisation()
					)
				) {
					return try_create_illegal_access_error().get();
				}

				return {};
			}
		);
	
	if(possible_resolved_field.is_unexpected()) {
		return move(possible_resolved_field.get_unexpected());
	}

	field& resolved_field = possible_resolved_field.get_expected();
	return try_put_field_resolved(resolved_field);
}