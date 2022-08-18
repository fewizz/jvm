#include "decl/lib/java/lang/invoke/var_handle.hpp"
#include "decl/classes.hpp"

static void init_java_lang_invoke_var_handle() {
	var_handle_class = classes.find_or_load(
		c_string{ "java/lang/invoke/VarHandle" }
	);
}