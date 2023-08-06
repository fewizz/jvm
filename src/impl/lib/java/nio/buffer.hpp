#include "decl/lib/java/nio/buffer.hpp"

#include "decl/classes.hpp"

static inline void init_java_nio_buffer() {
	buffer_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/nio/Buffer" }
	);
}