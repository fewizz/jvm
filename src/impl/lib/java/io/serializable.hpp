#include "decl/lib/java/io/serializable.hpp"

#include "classes.hpp"

static void init_java_io_serializable() {
	serializable_class = classes.find_or_load(
		c_string{ "java/io/Serializable" }
	);
}