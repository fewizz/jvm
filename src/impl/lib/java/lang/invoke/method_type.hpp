#include "lib/java/lang/invoke/method_type.hpp"
#include "class/load.hpp"

static void init_java_lang_invoke_method_type() {
	method_type_class = load_class(c_string{ "java/lang/invoke/MethodType" });

	method_type_return_type_instance_field_index = {
		method_type_class->find_instance_field_index(
			c_string{ "returnType_" },
			c_string{ "Ljava/lang/Class;" }
		)
	};

	method_type_parameter_types_instance_field_index = {
		method_type_class->find_instance_field_index(
			c_string{ "parameterTypes_" },
			c_string{ "[Ljava/lang/Class;" }
		)
	};
}