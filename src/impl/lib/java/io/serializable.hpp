#include "decl/lib/java/io/serializable.hpp"

#include "classes.hpp"

static void init_java_io_serializable() {
	serializable_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/io/Serializable" }
	);
}