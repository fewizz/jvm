#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <posix/io.hpp>

static void init_java_io_file_descriptor() {
	c& file_descriptor_class
		= classes.load_class_by_bootstrap_class_loader(
			u8"java/io/FileDescriptor"sv
		);

	file_descriptor_class.declared_static_methods().find(
		u8"stderr_fd"sv, u8"()I"sv
	).native_function(
		(void*)+[](native_environment*) {
			return posix::std_err.underlying();
		}
	);

	file_descriptor_class.declared_static_methods().find(
		u8"stdin_fd"sv, u8"()I"sv
	).native_function(
		(void*)+[](native_environment*) {
			return posix::std_in.underlying();
		}
	);

	file_descriptor_class.declared_static_methods().find(
		u8"stdout_fd"sv, u8"()I"sv
	).native_function(
		(void*)+[](native_environment*) {
			return posix::std_out.underlying();
		}
	);
}