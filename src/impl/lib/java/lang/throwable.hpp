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
		u8"java/lang/Throwable"sv
	);

	throwable_stack_trace_field_position =
		throwable_class->instance_field_position(
			u8"stackTrace_"sv,
			u8"[Ljava/lang/StackTraceElement;"sv
		);

	throwable_class->declared_instance_methods()
	.find(
		u8"fillInStackTrace"sv, u8"()Ljava/lang/Throwable;"sv
	).native_function((void*)+[](
		native_environment*, object* ths
	) -> object* {
		// lets skip constructors/fillInStackTrace frames
		execution_context* ctx_begin = latest_execution_context.ptr();
		auto this_class_name = ths->c().name();

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
				prev->method.c().name().has_equal_size_and_elements(
					this_class_name
				) &&
				prev->method.name().has_equal_size_and_elements(
					u8"<init>"sv
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
			thrown_in_native = possible_ste_array.move_unexpected();
			return nullptr;
		}
		reference ste_array = possible_ste_array.move_expected();

		execution_context* ctx = ctx_begin;

		for(reference& frame : array_as_span<reference>(ste_array)) {
			expected<reference, reference> possible_class_name
				= try_create_string_from_utf8(
					ctx->method.c().name()
				);
			
			if(possible_class_name.is_unexpected()) {
				thrown_in_native = possible_class_name.move_unexpected();
				return nullptr;
			}
			reference class_name = possible_class_name.move_expected();

			expected<reference, reference> possible_method_name
				= try_create_string_from_utf8(
					ctx->method.name()
				);

			if(possible_method_name.is_unexpected()) {
				thrown_in_native = possible_method_name.move_unexpected();
				return nullptr;
			}

			reference method_name = possible_method_name.move_expected();

			reference file_name{};
			uint16 line_number = -1;

			if(ctx->method.is_native()) {
				line_number = -2;
			}
			else if(ctx->method.c().has_source_file()) {
				expected<reference, reference> possible_file_name
					= try_create_string_from_utf8(
						ctx->method.c().source_file()
					);
				if(possible_file_name.is_unexpected()) {
					thrown_in_native = possible_file_name.move_unexpected();
					return nullptr;
				}
				file_name = possible_file_name.move_expected();

				uint32 pc =
					ctx->instruction_ptr - ctx->method.code().iterator();

				for(auto start_pc_and_line_number : ctx->method.line_numbers()){
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
				thrown_in_native = possible_ste.move_unexpected();
				return nullptr;
			}

			reference ste = possible_ste.move_expected();

			frame = move(ste);
			ctx = ctx->previous.ptr();
		}

		ths->set(throwable_stack_trace_field_position, move(ste_array));
		return ths;
	});

}

inline void j::throwable::init_cause(reference cause) {
	instance_method& m = throwable_class->declared_instance_methods().find(
		u8"initCause"sv,
		u8"(Ljava/lang/Throwable;)Ljava/lang/Throwable;"sv
	);
	optional<reference> possible_throwable = try_execute(m, *this, move(cause));
	if(possible_throwable.has_value()) {
		posix::abort();
	}
	stack.pop_back<reference>(); // pop this;
}