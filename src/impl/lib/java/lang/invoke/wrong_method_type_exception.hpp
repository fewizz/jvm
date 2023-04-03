#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"

#include "decl/classes.hpp"

static void init_java_lang_invoke_wrong_method_type_exception() {
	_class& c = classes.load_class_by_bootstrap_class_loader(
		c_string{"java/lang/invoke/WrongMethodTypeException"}
	);

	wrong_method_type_exception_constructor = c.declared_methods().find(
		c_string{"<init>"}, c_string{"()V"}
	);
}