#pragma once

#include "decl/class.hpp"
#include "decl/classes.hpp"
#include "decl/execution/latest_context.hpp"
#include "decl/object.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/invoke/method_handles_lookup.hpp"
#include "decl/lib/java/lang/illegal_caller_exception.hpp"

static void init_java_lang_invoke_method_handles() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/invoke/MethodHandles"sv
	);

	c.declared_static_methods().find(
		u8"lookup"sv,
		u8"()Ljava/lang/invoke/MethodHandles$Lookup;"sv
	).native_function(
		(void*)+[](native_environment*) -> object* {
			if(!latest_execution_context.has_value()) {
				thrown_in_native = try_create_illegal_caller_exception().get();
				return nullptr;
			}

			execution_context& prev_exe_context
				= latest_execution_context->previous.get();

			::c& caller_class = prev_exe_context.method.c();

			instance_method& constructor
				= method_handles_lookup_class->declared_instance_methods()
				.find(
					u8"<init>"sv,
					u8"(Ljava/lang/Class;)V"sv
				);

			expected<reference, reference> possible_lookup
				= try_create_object(
					constructor,
					reference{ caller_class.object() } // arg 0
				);
			if(possible_lookup.is_unexpected()) {
				thrown_in_native = possible_lookup.move_unexpected();
				return nullptr;
			}
			reference lookup = possible_lookup.move_expected();

			return & lookup.unsafe_release_without_destroing();
		}
	);
}