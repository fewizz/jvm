#include "decl/lib/java/lang/byte.hpp"

#include "classes.hpp"

static void init_java_lang_byte() {
	java_lang_byte_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Byte" }
	);

	java_lang_byte_constructor =
		java_lang_byte_class->declared_instance_methods()
		.find(c_string{"<init>"}, c_string{"(B)V"});
}