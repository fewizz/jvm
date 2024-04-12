#include "decl/lib/java/lang/stack_trace_element.hpp"

#include "decl/classes.hpp"

static inline void init_java_lang_stack_trace_element() {
	stack_trace_element_class = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/StackTraceElement"s
	);

	stack_trace_element_constructor =
		stack_trace_element_class->instance_methods().find(
			u8"<init>"s,
			"("
				"Ljava/lang/String;"
				"Ljava/lang/String;"
				"Ljava/lang/String;"
				"I"
			")V"s
		);
	
}