#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "decl/class/load.hpp"
#include "decl/native/functions.hpp"

static void init_java_lang_invoke_method_handle() {
	method_handle_class = load_class(
		c_string{ "java/lang/invoke/MethodHandle" }
	);

	method_handle_class_instance_field_index = {
		method_handle_class->instance_fields().find_index_of(
			c_string{ "class_" }, c_string{ "J" }
		)
	};

	method_handle_member_instance_field_index = {
		method_handle_class->instance_fields().find_index_of(
			c_string{ "member_" }, c_string{ "J" }
		)
	};

	method_handle_kind_instance_field_index = {
		method_handle_class->instance_fields().find_index_of(
			c_string{ "kind_" }, c_string{ "B" }
		)
	};
}