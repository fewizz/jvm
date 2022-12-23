#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <posix/io.hpp>

static void init_java_io_file_descriptor() {
	_class& file_descriptor_class = classes.find_or_load(
		c_string{ "java/io/FileDescriptor" }
	);

	file_descriptor_class.declared_methods().find(
		c_string{ "stderr_fd" }, c_string{ "()I" }
	).native_function(
		(void*)+[](native_environment*) {
			return posix::std_err.underlying();
		}
	);

	file_descriptor_class.declared_methods().find(
		c_string{ "stdin_fd" }, c_string{ "()I" }
	).native_function(
		(void*)+[](native_environment*) {
			return posix::std_in.underlying();
		}
	);

	file_descriptor_class.declared_methods().find(
		c_string{ "stdout_fd" }, c_string{ "()I" }
	).native_function(
		(void*)+[](native_environment*) {
			return posix::std_out.underlying();
		}
	);
}