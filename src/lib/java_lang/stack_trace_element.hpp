#pragma once

#include "class/decl.hpp"
#include "classes/find_or_load.hpp"
#include "lib/java_lang/string.hpp"

#include <core/meta/elements/optional.hpp>
#include <core/c_string.hpp>

static optional<_class&> stack_trace_element_class;
static optional<method&> stack_trace_element_constructor;

template<range ClassName, range MethodName>
static inline reference create_stack_trace_element(
	ClassName&& class_name, MethodName&& method_name
) {
	reference class_name_str = create_string_from_utf8(class_name);
	reference method_name_str = create_string_from_utf8(method_name);

	reference result = create_object(stack_trace_element_class.value());

	stack_entry args[] {
		result, class_name_str, method_name_str, reference{}, jint{ -1 }
	};

	execute(
		method_with_class {
			stack_trace_element_constructor.value(),
			stack_trace_element_class.value()
		},
		args_container{ args }
	);

	return move(result);
}

static inline void init_java_lang_stack_trace_element() {
	stack_trace_element_class =
		load_class(c_string{ "java/lang/StackTraceElement" });

	stack_trace_element_constructor =
		stack_trace_element_class->find_method(
			c_string{ "<init>" },
			c_string {
				"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V"
			}
		);
	
}