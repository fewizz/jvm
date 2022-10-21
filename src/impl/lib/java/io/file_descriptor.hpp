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
		(void*) (int32 (*)(native_environment*))
		[](native_environment*) {
			return 2;
		}
	);

	file_descriptor_class.declared_methods().find(
		c_string{ "stdin_fd" }, c_string{ "()I" }
	).native_function(
		(void*) (int32 (*)(native_environment*))
		[](native_environment*) {
			return 0;
		}
	);

	file_descriptor_class.declared_methods().find(
		c_string{ "stdout_fd" }, c_string{ "()I" }
	).native_function(
		(void*) (int32 (*)(native_environment*))
		[](native_environment*) {
			return 1;
		}
	);
}