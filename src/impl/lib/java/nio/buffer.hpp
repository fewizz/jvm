#include "decl/lib/java/nio/buffer.hpp"

#include "decl/class/load.hpp"

static inline void init_java_nio_buffer() {
	buffer_class = load_class(c_string{ "java/nio/Buffer" });
}