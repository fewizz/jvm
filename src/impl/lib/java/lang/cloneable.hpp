#include "decl/lib/java/lang/cloneable.hpp"

#include "decl/classes.hpp"

static void init_java_lang_cloneable() {
	cloneable_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Cloneable" }
	);
}