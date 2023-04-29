#include "decl/reference.hpp"
#include "decl/class.hpp"
#include "decl/execution/stack.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"

#include <class_file/constant.hpp>

template<typename Type>
inline Type& get_static_resolved(
	static_field& resolved_field
) {
	declared_static_field_index index =
		resolved_field._class().declared_static_fields()
		.find_index_of(resolved_field);

	return resolved_field._class().get<Type>(index);
}

inline void get_static_resolved(
	static_field& resolved_field
) {
	declared_static_field_index index =
		resolved_field._class().declared_static_fields()
		.find_index_of(resolved_field);

	resolved_field._class().view(index, [](auto& field_value) {
		stack.emplace_back(field_value);
	});
}

[[nodiscard]] inline optional<reference> try_get_static(
	_class& d, class_file::constant::field_ref_index ref_index
) {
	/* The referenced field is resolved (§5.4.3.2). */
	expected<field&, reference> possible_resolved_field
		= d.try_get_resolved_field(
			ref_index,
			[&](field& f) -> optional<reference> {
				/* Otherwise, if the resolved field is not a static (class)
				   field or an interface field, getstatic throws an
				   IncompatibleClassChangeError. */
				if(!f.is_static()) {
					return try_create_incompatible_class_change_error().get();
				}

				/* On successful resolution of the field, the class or interface
				   that declared the resolved field is initialized if that class
				   or interface has not already been initialized (§5.5). */
				optional<reference> possible_throwable
					= f._class().try_initialise_if_need();
				if(possible_throwable.has_value()) {
					return possible_throwable.move();
				}
				return {};
			}
		);

	if(possible_resolved_field.is_unexpected()) {
		return possible_resolved_field.move_unexpected();
	}

	static_field& resolved_field =
		(static_field&) possible_resolved_field.get_expected();

	get_static_resolved(resolved_field);

	return {};
}