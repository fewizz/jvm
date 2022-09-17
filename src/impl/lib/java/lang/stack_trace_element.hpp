#include "decl/lib/java/lang/stack_trace_element.hpp"

#include "decl/class.hpp"
#include "decl/classes.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/execute.hpp"

template<basic_range ClassName, basic_range MethodName>
static inline reference create_stack_trace_element(
	ClassName&& class_name, MethodName&& method_name
) {
	reference class_name_str  = create_string_from_utf8(class_name);
	reference method_name_str = create_string_from_utf8(method_name);

	reference result = create_object(stack_trace_element_class.value());

	stack_entry args[] {
		result, move(class_name_str), move(method_name_str),
		reference{}, jint{ -1 }
	};

	execute(stack_trace_element_constructor.value(), arguments_span{ args });

	return result;
}

static inline void init_java_lang_stack_trace_element() {
	stack_trace_element_class = classes.find_or_load(
		c_string{ "java/lang/StackTraceElement" }
	);

	stack_trace_element_constructor = *
		stack_trace_element_class->instance_methods().find(
			c_string{ "<init>" },
			c_string {
				"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V"
			}
		);
	
}