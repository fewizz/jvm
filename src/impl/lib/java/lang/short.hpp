#include "decl/lib/java/lang/short.hpp"

#include "classes.hpp"

static void init_java_lang_short() {
	java_lang_short_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Short" }
	);

	java_lang_short_constructor =
		java_lang_short_class->declared_instance_methods()
		.find(c_string{"<init>"}, c_string{"(S)V"});
}