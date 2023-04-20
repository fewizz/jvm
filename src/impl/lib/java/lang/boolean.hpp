#include "decl/lib/java/lang/boolean.hpp"

#include "classes.hpp"

static void init_java_lang_boolean() {
	java_lang_boolean_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Boolean" }
	);

	java_lang_boolean_constructor =
		java_lang_boolean_class->declared_instance_methods()
		.find(c_string{"<init>"}, c_string{"(Z)V"});
}