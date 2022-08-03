#include "lib/java/lang/invoke/method_handle.hpp"

#include "class/load.hpp"
#include "native/functions.hpp"

static void init_java_lang_invoke_method_handle() {
	method_handle_class = load_class(
		c_string{ "java/lang/invoke/MethodHandle" }
	);

	method_handle_class_instance_field_index = {
		method_handle_class->find_instance_field_index(
			c_string{ "class_" }, c_string{ "J" }
		)
	};

	method_handle_member_instance_field_index = {
		method_handle_class->find_instance_field_index(
			c_string{ "member_" }, c_string{ "J" }
		)
	};

	method_handle_kind_instance_field_index = {
		method_handle_class->find_instance_field_index(
			c_string{ "kind_" }, c_string{ "B" }
		)
	};
}