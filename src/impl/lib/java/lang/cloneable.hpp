#include "decl/lib/java/lang/cloneable.hpp"

#include "decl/classes.hpp"

static void init_java_lang_cloneable() {
	cloneable_class = classes.find_or_load(c_string{ "java/lang/Cloneable" });
}