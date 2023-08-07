#include "decl/class/layout.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <posix/io.hpp>

static layout::position file_input_stream_fd_field_position;

static void init_java_io_file_input_stream() {
	c& c
		= classes.load_class_by_bootstrap_class_loader(
			c_string{ u8"java/io/FileInputStream" }
		);

	file_input_stream_fd_field_position =
		c.instance_field_position(
			c_string{ u8"fd_" }, c_string{ u8"I" }
		);

	c.declared_instance_methods().find(
		c_string{ u8"read" }, c_string{ u8"()I" }
	).native_function(
		(void*)+[](native_environment*, object* ths) -> int32 {
			handle<posix::file> fd {
				ths->get<int32>(file_input_stream_fd_field_position)
			};
			uint8 ch;
			nuint read = fd->try_read_to(
				span{&ch},
				[](auto){ posix::abort(); }
			);
			if(read == 0) {
				return -1;
			}
			return ch;
		}
	);
}