#include "decl/lib/java/lang/invoke/var_handle.hpp"
#include "decl/class/load.hpp"

static void init_java_lang_invoke_var_handle() {
	var_handle_class = load_class(c_string{ "java/lang/invoke/VarHandle" });
}