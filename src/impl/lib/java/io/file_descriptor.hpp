#include "decl/classes.hpp"
#include "decl/native/interface/environment.hpp"

#include <stdio.h>

static void init_java_io_file_descriptor() {
	_class& file_descriptor_class = classes.find_or_load(
		c_string{ "java/io/FileDescriptor" }
	);

	file_descriptor_class.declared_methods().find(
		c_string{ "stderr_fd" }, c_string{ "()J" }
	).native_function(
		(void*) (int64 (*)(native_interface_environment*)) [](native_interface_environment*) {
			return (int64) stderr;
		}
	);

	file_descriptor_class.declared_methods().find(
		c_string{ "stdin_fd" }, c_string{ "()J" }
	).native_function(
		(void*) (int64 (*)(native_interface_environment*)) [](native_interface_environment*) {
			return (int64) stdin;
		}
	);

	file_descriptor_class.declared_methods().find(
		c_string{ "stdout_fd" }, c_string{ "()J" }
	).native_function(
		(void*) (int64 (*)(native_interface_environment*)) [](native_interface_environment*) {
			return (int64) stdout;
		}
	);
}