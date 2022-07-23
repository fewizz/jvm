#pragma once

#include "native/jni/environment.hpp"
#include "native/functions/container.hpp"
#include "execution/latest_context.hpp"
#include "array.hpp"
#include "primitives.hpp"
#include "object/reference/decl.hpp"
#include "lib/java_lang/stack_trace_element.hpp"

#include <core/meta/elements/optional.hpp>

static optional<_class&> throwable_class;
static instance_field_index throwable_stack_trace_field_index;

static inline void init_java_lang_throwable() {

	throwable_class = load_class(c_string{ "java/lang/Throwable" });

	throwable_stack_trace_field_index =
		throwable_class->find_instance_field_index(
			c_string{ "stackTrace_" },
			c_string{ "[Ljava/lang/StackTraceElement;" }
		);

	native_functions.emplace_back(
		(void*) (object* (*)(jni_environment*, object*))
		[](jni_environment*, object* ths) {
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
					ctx->_class.name(ctx->method)
				);
				ctx = ctx->previous.ptr();
			}

			ths->values()[throwable_stack_trace_field_index] = ste_array;
			return ths;
		},
		c_string{ "Java_java_lang_Throwable_fillInStackTrace" },
		c_string{ "()Ljava/lang/Throwable;" }
	);

}