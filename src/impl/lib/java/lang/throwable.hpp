#include "decl/lib/java/lang/throwable.hpp"

#include "decl/native/interface/environment.hpp"
#include "decl/execution/latest_context.hpp"
#include "decl/array.hpp"
#include "decl/primitives.hpp"
#include "decl/object/reference.hpp"
#include "decl/lib/java/lang/stack_trace_element.hpp"
#include "decl/class/load.hpp"
#include "decl/classes.hpp"

static inline void init_java_lang_throwable() {

	throwable_class = classes.find_or_load(c_string{ "java/lang/Throwable" });

	throwable_stack_trace_field_index =
		throwable_class->instance_fields().find_index_of(
			c_string{ "stackTrace_" },
			c_string{ "[Ljava/lang/StackTraceElement;" }
		);


	throwable_class->declared_instance_methods()
	.find(
		c_string{ "fillInStackTrace" }, c_string{ "()Ljava/lang/Throwable;" }
	).native_function(
		(void*) (object* (*)(native_interface_environment*, object*))
		[](native_interface_environment*, object* ths) {
			execution_context* ctx = latest_execution_context.ptr();
			int32 frames_count = 0;

			while(ctx != nullptr) {
				ctx = ctx->previous.ptr();
				++frames_count;
			}

			reference ste_array = create_array_of(
				stack_trace_element_class.value(), frames_count
			);
			reference* data = array_data<reference>(ste_array.object());

			ctx = latest_execution_context.ptr();

			for(int32 x = 0; x < frames_count; ++x) {
				data[x] = create_stack_trace_element(
					ctx->_class.name(),
					ctx->method.name()
				);
				ctx = ctx->previous.ptr();
			}

			ths->values()[throwable_stack_trace_field_index] = move(ste_array);
			return ths;
		}
	);

}