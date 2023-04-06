#pragma once

#include "decl/class.hpp"
#include "decl/classes.hpp"
#include "decl/execution/latest_context.hpp"
#include "decl/execute.hpp"
#include "decl/object.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/invoke/method_handles_lookup.hpp"

static void init_java_lang_invoke_method_handles() {
	_class& c = classes.load_class_by_bootstrap_class_loader(
		c_string{"java/lang/invoke/MethodHandles"}
	);

	c.declared_static_methods().find(
		c_string{"lookup"},
		c_string{"()Ljava/lang/invoke/MethodHandles$Lookup;"}
	).native_function(
		(void*)+[](native_environment*) -> object* {
			execution_context& prev_exe_context
				= latest_execution_context->previous.get();

			expected<reference, reference> possible_lookup
				= try_create_object(method_handles_lookup_class.get());
			if(possible_lookup.is_unexpected()) {
				thrown_in_native = move(possible_lookup.get_unexpected());
				return nullptr;
			}
			reference lookup = move(possible_lookup.get_expected());
			stack.emplace_back(lookup); // this

			_class& caller_class = prev_exe_context.method._class();
			stack.emplace_back(caller_class.instance()); // arg 0

			method& constructor
				= method_handles_lookup_class->declared_methods()
				.find(c_string{"<init>"}, c_string{"(Ljava/lang/Class;)V"});
			
			optional<reference> possible_throwable = try_execute(constructor);
			if(possible_throwable.has_value()) {
				thrown_in_native = move(possible_throwable.get());
				return nullptr;
			}

			return & lookup.unsafe_release_without_destroing();
		}
	);
}