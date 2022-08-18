#include "decl/lib/java/nio/buffer.hpp"

#include "decl/classes.hpp"

static inline void init_java_nio_buffer() {
	buffer_class = classes.find_or_load(c_string{ "java/nio/Buffer" });
}