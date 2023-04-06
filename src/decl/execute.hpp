#pragma once

#include "execution/stack.hpp"
#include "method.hpp"

[[nodiscard]] static optional<reference> try_execute(method& m);

template<typename... Args>
[[nodiscard]] optional<reference> try_execute(method& m, Args&&... args) {
	(stack.emplace_back(forward<Args>(args)), ...);
	return try_execute(m);
}

template<basic_range StackType>
[[nodiscard]] inline optional<reference> try_invoke_dynamic(
	class_file::constant::invoke_dynamic_index ref_index, _class& c
);

template<basic_range StackType>
[[nodiscard]] inline optional<reference> try_invoke_static(
	class_file::constant::method_ref_index ref_index, _class& c
);

// 5.4.6. Method Selection
/* "During execution of an invokeinterface or invokevirtual instruction,
    a method is selected with respect to (i) the run-time type of the object on
    the stack, and (ii) a method that was previously resolved by the
    instruction. The rules to select a method with respect to a class or
    interface C and a method mR are as follows:" */
method& select_method(
	_class& c, method& mr
);

method& select_method_for_invoke_special(
	_class& current, _class& referenced_class, method& resolved_method
);