#include "decl/classes.hpp"
#include "decl/object.hpp"

static void init_java_lang_invoke_string_concat_factory() {
	_class& scf = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/invoke/StringConcatFactory" }
	);
	method& make_concat_with_constants = scf.declared_methods().find(
		c_string{ "makeConcatWithConstants" },
		c_string {
			"("
				"Ljava/lang/invoke/MethodHandles$Lookup;"
				"Ljava/lang/String;"
				"Ljava/lang/invoke/MethodType;"
				"Ljava/lang/String;"
				"[Ljava/lang/Object;"
			")"
			"Ljava/lang/invoke/CallSite;"
		}
	);

	make_concat_with_constants.native_function(
		(void*)+[](
			[[maybe_unused]] object* lookup,
			object*,
			object*,
			object*,
			object*
		) -> object* {
			posix::abort();
		}
	);
}