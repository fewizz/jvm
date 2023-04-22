#include "decl/reference.hpp"
#include "decl/class.hpp"
#include "decl/execution/stack.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"
#include "decl/lib/java/lang/illegal_access_error.hpp"

#include <class_file/constant.hpp>

inline void put_static_resolved(field& resolved_field) {
	declared_static_field_index index =
		resolved_field._class().declared_static_fields()
		.find_index_of(resolved_field);

	resolved_field._class().view(
		index,
		[&]<typename FieldType>(FieldType& field_value) {
			/* If the value is of type int and the field descriptor type is
			   boolean, then the int value is narrowed by taking the bitwise AND
			   of value and 1, resulting in value'. The referenced field in the
			   class or interface is set to value'. */
			// done automagically?
			field_value = stack.pop_back<FieldType>();
		}
	);
}

[[nodiscard]] inline optional<reference> try_put_static(
	method& current_method,
	class_file::constant::field_ref_index ref_index
) {
	_class& d = current_method._class();

	/* The referenced field is resolved (§5.4.3.2). */
	expected<field&, reference> possible_resolved_field
		= d.try_get_resolved_field(
			ref_index,
			[&](field& f) -> optional<reference> {
				/* Otherwise, if the resolved field is not a static (class)
				   field or an interface field, putstatic throws an
				   IncompatibleClassChangeError. */
				if(!f.is_static()) {
					return try_create_incompatible_class_change_error().get();
				}
				/* Otherwise, if the resolved field is final, it must be
				   declared in the current class or interface, and the
				   instruction must occur in the class or interface
				   initialization method of the current class or interface.
				   Otherwise, an IllegalAccessError is thrown. */
				
				if(
					f.is_final() && !(
						f._class().is(d) &&
						current_method.is_class_initialisation()
					)
				) {
					return try_create_illegal_access_error().get();
				}
				return {};
			}
		);
	

	if(possible_resolved_field.is_unexpected()) {
		return possible_resolved_field.move_unexpected();
	}

	field& resolved_field = possible_resolved_field.get_expected();

	put_static_resolved(resolved_field);

	return {};
}