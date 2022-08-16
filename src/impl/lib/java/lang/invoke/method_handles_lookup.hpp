#include "decl/native/functions.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/string.hpp"

static void init_java_lang_invoke_method_handles_lookup() {
	native_functions.emplace_back(
		(void*)
		(object*(*)(jni_environment*, object*, object*, object*, object*))
		[](
			jni_environment*, object*,
			object*, object*, object*
		) -> object* {
			//_class& c = class_from_class_instance(*refc);
			// TODO
			return nullptr;
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