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

// s - class type of objectref, t - resolved class
inline bool can_cast(_class& s, _class& t);

// invoke_dynamic

template<basic_range StackType>
[[nodiscard]] inline optional<reference> try_invoke_dynamic(
	class_file::constant::invoke_dynamic_index ref_index, _class& c
);

template<basic_range StackType>
[[nodiscard]] inline optional<reference> try_invoke_static(
	class_file::constant::method_ref_index ref_index, _class& c
);

// invoke_special

[[nodiscard]] inline optional<reference> try_invoke_special_selected(
	method& selected_method
);

[[nodiscard]] inline optional<reference> try_invoke_special_resolved(
	_class& current_c, method& resolved_method
);

[[nodiscard]] inline optional<reference> try_invoke_special(
	_class& current_c,
	class_file::constant::method_or_interface_method_ref_index ref_index
);

[[nodiscard]] optional<method&> try_select_method(
	_class& c, method& mr
);

[[nodiscard]] optional<instance_method&>
select_method_for_invoke_special(
	_class& current, _class& referenced_class, method& resolved_method
);

// invoke_virtual

[[nodiscard]] inline optional<reference>
try_invoke_virtual_resolved_non_polymorphic(
	method& resolved_method
);

[[nodiscard]] inline optional<reference> try_invoke_virtual(
	_class& d, class_file::constant::method_ref_index ref_index
);

// invoke_static

[[nodiscard]] inline optional<reference>
try_invoke_static_resolved(static_method& resolved_method);

inline optional<reference> try_invoke_static(
	_class& d,
	class_file::constant::method_or_interface_method_ref_index ref_index
);

// invoke_interface

[[nodiscard]] inline optional<reference> try_invoke_interface(
	_class& d, class_file::constant::interface_method_ref_index ref_index
);

// invoke_dynamic

[[nodiscard]] inline optional<reference> try_invoke_dynamic(
	_class& d, class_file::constant::invoke_dynamic_index ref_index
);

// get_field

[[nodiscard]] inline optional<reference> try_get_field_resolved(
	instance_field_index resolved_field_index
);

[[nodiscard]] inline optional<reference> try_get_field_resolved(
	instance_field& resolved_field
);

[[nodiscard]] inline optional<reference> try_get_field(
	_class& d, class_file::constant::field_ref_index ref_index
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
	_class& d, class_file::constant::field_ref_index ref_index
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