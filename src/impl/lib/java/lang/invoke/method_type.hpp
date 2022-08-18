#include "decl/lib/java/lang/invoke/method_type.hpp"
#include "decl/classes.hpp"

static void init_java_lang_invoke_method_type() {
	method_type_class = classes.find_or_load(
		c_string{ "java/lang/invoke/MethodType" }
	);

	method_type_return_type_instance_field_index = {
		method_type_class->instance_fields().find_index_of(
			c_string{ "returnType_" },
			c_string{ "Ljava/lang/Class;" }
		)
	};

	method_type_parameter_types_instance_field_index = {
		method_type_class->instance_fields().find_index_of(
			c_string{ "parameterTypes_" },
			c_string{ "[Ljava/lang/Class;" }
		)
	};
}