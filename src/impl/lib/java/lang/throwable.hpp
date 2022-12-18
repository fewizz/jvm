#include "decl/lib/java/lang/throwable.hpp"

#include "decl/native/environment.hpp"
#include "decl/execution/latest_context.hpp"
#include "decl/array.hpp"
#include "decl/primitives.hpp"
#include "decl/reference.hpp"
#include "decl/lib/java/lang/stack_trace_element.hpp"
#include "decl/class/load.hpp"
#include "decl/classes.hpp"

static inline void init_java_lang_throwable() {

	throwable_class = classes.find_or_load(c_string{ "java/lang/Throwable" });

	throwable_stack_trace_field_position =
		throwable_class->instance_field_position(
			c_string{ "stackTrace_" },
			c_string{ "[Ljava/lang/StackTraceElement;" }
		);

	throwable_class->declared_instance_methods()
	.find(
		c_string{ "fillInStackTrace" }, c_string{ "()Ljava/lang/Throwable;" }
	).native_function(
		(void*) (object* (*)(native_environment*, object*))
		[](native_environment*, object* ths) {
			// lets skip constructors/fillInStackTrace frames
			execution_context* ctx_begin = latest_execution_context.ptr();
			auto this_class_name = ths->_class().name();

			while(true) {
				execution_context* prev = ctx_begin;

				ctx_begin = ctx_begin->previous.ptr();
				if(ctx_begin == nullptr) {
					// didn't find
					abort();
				}

				if(
					prev->method._class().name().have_elements_equal_to(
						this_class_name
					) &&
					prev->method.name().have_elements_equal_to(
						c_string{ "<init>" }
					)
				) {
					break;
				}
			}

			nuint frames_count = ctx_begin->frames_until_end();

			reference ste_array = create_array_of(
				stack_trace_element_class.get(), frames_count
			);

			execution_context* ctx = ctx_begin;

			for(reference& frame : array_as_span<reference>(ste_array)) {
				frame = create_stack_trace_element(
					ctx->method._class().name(),
					ctx->method.name()
				);
				ctx = ctx->previous.ptr();
			}

			ths->set(throwable_stack_trace_field_position, move(ste_array));
			return ths;
		}
	);

}