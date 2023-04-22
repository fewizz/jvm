#include "decl/lib/java/lang/throwable.hpp"

#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/execution/latest_context.hpp"
#include "decl/execute.hpp"
#include "decl/array.hpp"
#include "decl/primitives.hpp"
#include "decl/reference.hpp"
#include "decl/lib/java/lang/stack_trace_element.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/classes.hpp"

static inline void init_java_lang_throwable() {

	throwable_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Throwable" }
	);

	throwable_stack_trace_field_position =
		throwable_class->instance_field_position(
			c_string{ "stackTrace_" },
			c_string{ "[Ljava/lang/StackTraceElement;" }
		);

	throwable_class->declared_instance_methods()
	.find(
		c_string{ "fillInStackTrace" }, c_string{ "()Ljava/lang/Throwable;" }
	).native_function((void*)+[](native_environment*, object* ths) -> object* {
		// lets skip constructors/fillInStackTrace frames
		execution_context* ctx_begin = latest_execution_context.ptr();
		auto this_class_name = ths->_class().name();

		while(true) {
			execution_context* prev = ctx_begin;

			// isn't possible
			if(ctx_begin == nullptr) {
				posix::abort();
			}

			ctx_begin = ctx_begin->previous.ptr();
			if(ctx_begin == nullptr) {
				// didn't find
				posix::abort();
			}

			if(
				prev->method._class().name().has_equal_size_and_elements(
					this_class_name
				) &&
				prev->method.name().has_equal_size_and_elements(
					c_string{ "<init>" }
				)
			) {
				break;
			}
		}

		nuint frames_count = ctx_begin->frames_until_end();

		expected<reference, reference> possible_ste_array = try_create_array_of(
			stack_trace_element_class.get(), frames_count
		);
		if(possible_ste_array.is_unexpected()) {
			thrown_in_native = move(possible_ste_array.get_unexpected());
			return nullptr;
		}
		reference ste_array = move(possible_ste_array.get_expected());

		execution_context* ctx = ctx_begin;

		for(reference& frame : array_as_span<reference>(ste_array)) {
			expected<reference, reference> possible_class_name
				= try_create_string_from_utf8(
					ctx->method._class().name()
				);
			
			if(possible_class_name.is_unexpected()) {
				thrown_in_native = move(possible_class_name.get_unexpected());
				return nullptr;
			}
			reference class_name = move(possible_class_name.get_expected());

			expected<reference, reference> possible_method_name
				= try_create_string_from_utf8(
					ctx->method.name()
				);

			if(possible_method_name.is_unexpected()) {
				thrown_in_native = move(possible_method_name.get_unexpected());
				return nullptr;
			}

			reference method_name = move(possible_method_name.get_expected());

			reference file_name{};
			uint16 line_number = -1;

			if(ctx->method.is_native()) {
				line_number = -2;
			}
			else if(ctx->method._class().has_source_file()) {
				expected<reference, reference> possible_file_name
					= try_create_string_from_utf8(
						ctx->method._class().source_file()
					);
				if(possible_file_name.is_unexpected()) {
					thrown_in_native
						= move(possible_file_name.get_unexpected());
					return nullptr;
				}
				file_name = move(possible_file_name.get_expected());

				uint32 pc =
					ctx->instruction_ptr - ctx->method.code().iterator();

				for(
					auto start_pc_and_line_number :
					ctx->method.line_numbers().as_span()
				) {
					auto [start_pc, possible_line_number]
						= start_pc_and_line_number;
					if(start_pc <= pc) {
						line_number = (uint16) possible_line_number;
					}
				}
			}

			expected<reference, reference> possible_ste
				= try_create_object(
					stack_trace_element_constructor.get(),
					move(class_name),
					move(method_name),
					move(file_name), // file name
					int32{ line_number }
				);
			if(possible_ste.is_unexpected()) {
				thrown_in_native = move(possible_ste.get_unexpected());
				return nullptr;
			}

			reference ste = move(possible_ste.get_expected());

			frame = move(ste);
			ctx = ctx->previous.ptr();
		}

		ths->set(throwable_stack_trace_field_position, move(ste_array));
		return ths;
	});

}