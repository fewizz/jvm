#include "decl/lib/java/lang/invoke/var_handle.hpp"
#include "decl/classes.hpp"

static void init_java_lang_invoke_var_handle() {
	var_handle_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/invoke/VarHandle" }
	);
}