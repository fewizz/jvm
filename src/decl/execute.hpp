#pragma once

#include "execution/stack.hpp"
#include "method.hpp"
#include "field.hpp"

[[nodiscard]] static optional<reference> try_execute(method& m);

template<typename... Args>
[[nodiscard]] optional<reference> try_execute(method& m, Args&&... args) {
	(stack.emplace_back(forward<Args>(args)), ...);
	return try_execute(m);
}

inline optional<reference>
try_check_cast(reference& from, c& to);

// s - class type of objectref, t - resolved class
inline bool can_cast(c& s, c& t);

// invoke_dynamic

template<basic_range StackType>
[[nodiscard]] inline optional<reference> try_invoke_dynamic(
	class_file::constant::invoke_dynamic_index ref_index, c& c
);

template<basic_range StackType>
[[nodiscard]] inline optional<reference> try_invoke_static(
	class_file::constant::method_ref_index ref_index, c& c
);

// invoke_special

[[nodiscard]] inline optional<reference> try_invoke_special_selected(
	instance_method& selected_method
);

[[nodiscard]] inline optional<reference> try_invoke_special_resolved(
	c& current_c, instance_method& resolved_method
);

[[nodiscard]] inline optional<reference> try_invoke_special(
	c& current_c,
	class_file::constant::method_or_interface_method_ref_index ref_index
);

[[nodiscard]] optional<method&> try_select_method(
	c& c, method& mr
);

[[nodiscard]] optional<instance_method&>
select_method_for_invoke_special(
	c& current, c& referenced_class, method& resolved_method
);

// invoke_virtual

[[nodiscard]] inline optional<reference>
try_invoke_virtual_resolved_non_polymorphic(
	method& resolved_method
);

[[nodiscard]] inline optional<reference> try_invoke_virtual(
	c& d, class_file::constant::method_ref_index ref_index
);

// invoke_static

[[nodiscard]] inline optional<reference>
try_invoke_static_resolved(static_method& resolved_method);

inline optional<reference> try_invoke_static(
	c& d,
	class_file::constant::method_or_interface_method_ref_index ref_index
);

// invoke_interface

[[nodiscard]] inline optional<reference> try_invoke_interface(
	c& d, class_file::constant::interface_method_ref_index ref_index
);

// invoke_dynamic

[[nodiscard]] inline optional<reference> try_invoke_dynamic(
	c& d, class_file::constant::invoke_dynamic_index ref_index
);

// get_field

[[nodiscard]] inline optional<reference> try_get_field_resolved(
	instance_field_index resolved_field_index
);

[[nodiscard]] inline optional<reference> try_get_field_resolved(
	instance_field& resolved_field
);

[[nodiscard]] inline optional<reference> try_get_field(
	c& d, class_file::constant::field_ref_index ref_index
);

template<typename Type>
[[nodiscard]] inline expected<Type, reference> try_get_field_resolved(
	instance_field& resolved_field, reference ref
) {
	stack.emplace_back(move(ref));
	optional<reference> possible_throwable
		= try_get_field_resolved(resolved_field);
	if(possible_throwable.has_value()) {
		return unexpected{ possible_throwable.move() };
	}
	Type result = stack.pop_back<Type>();
	return move(result);
}

// put_field

[[nodiscard]] inline optional<reference>
try_put_field(
	method& current_method,
	class_file::constant::field_ref_index ref_index
);

[[nodiscard]] inline optional<reference>
try_put_field_resolved(instance_field& resolved_field);

// get_static

[[nodiscard]] inline optional<reference> try_get_static(
	c& d, class_file::constant::field_ref_index ref_index
);

inline void get_static_resolved(
	static_field& resolved_field
);

template<typename Type>
inline Type& get_static_resolved(
	static_field& resolved_field
);

// put_static

[[nodiscard]] inline optional<reference> try_put_static(
	method& current_method,
	class_file::constant::field_ref_index ref_index
);

inline void put_static_resolved(
	static_field& resolved_field
);