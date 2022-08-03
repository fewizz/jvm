#include "native/functions.hpp"
#include "lib/java/lang/class.hpp"
#include "lib/java/lang/string.hpp"

static void init_java_lang_invoke_method_handles_lookup() {
	native_functions.emplace_back(
		(void*)
		(object*(*)(jni_environment*, object*, object*, object*, object*))
		[](
			jni_environment*, object* ths,
			object* refc, object* name, object* type
		) -> object* {
			_class& c = class_from_class_instance(*refc);
		},
		c_string { "Java_java_lang_invoke_MethodHandles$findStatic" },
		c_string {
			"("
				"Ljava/lang/Class;"
				"Ljava/lang/String;"
				"Ljava/lang/invoke/MethodType;"
			")"
			"Ljava/lang/invoke/MethodHandle;"
		}
	);
}