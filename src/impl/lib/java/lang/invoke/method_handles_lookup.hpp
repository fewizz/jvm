#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/classes.hpp"
#include "decl/native/interface/environment.hpp"

static void init_java_lang_invoke_method_handles_lookup() {
	_class& method_handles_lookup_class = classes.find_or_load(
		c_string{ "java/lang/invoke/MethodHandles$Lookup" }
	);

	method_handles_lookup_class.declared_methods().find(
		c_string{ "findStatic" },
		c_string {
			"("
				"Ljava/lang/Class;"
				"Ljava/lang/String;"
				"Ljava/lang/invoke/MethodType;"
			")"
			"Ljava/lang/invoke/MethodHandle;"
		}
	).native_function(
		(void*)
		(object*(*)(
			native_interface_environment*, object*, object*, object*, object*
		))
		[](
			native_interface_environment*, object*,
			object*, object*, object*
		) -> object* {
			//_class& c = class_from_class_instance(*refc);
			// TODO
			return nullptr;
		}
	);
}