#include "decl/lib/java/lang/stack_trace_element.hpp"

#include "decl/class.hpp"
#include "decl/classes.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/execute.hpp"

static inline void init_java_lang_stack_trace_element() {
	stack_trace_element_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/StackTraceElement" }
	);

	stack_trace_element_constructor =
		stack_trace_element_class->instance_methods().find(
			c_string{ "<init>" },
			c_string {
				"("
					"Ljava/lang/String;"
					"Ljava/lang/String;"
					"Ljava/lang/String;"
					"I"
				")V"
			}
		);
	
}