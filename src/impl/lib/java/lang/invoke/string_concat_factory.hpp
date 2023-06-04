#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/native/environment.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"
#include "decl/lib/jvm/mh/string_concat.hpp"

static void init_java_lang_invoke_string_concat_factory() {
	c& scf = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/invoke/StringConcatFactory" }
	);
	method& make_concat_with_constants = scf.declared_static_methods().find(
		c_string{ u8"makeConcatWithConstants" },
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
			native_environment*,
			object*,
			j::string*,
			j::method_type*,
			j::string*,
			object*
		) -> object* {
			
		}
	);
}