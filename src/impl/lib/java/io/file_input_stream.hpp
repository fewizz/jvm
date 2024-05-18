#include "decl/class/layout.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <posix/io.hpp>

static layout::position file_input_stream_fd_field_position;

static void init_java_io_file_input_stream() {
	c& c
		= classes.load_class_by_bootstrap_class_loader(
			u8"java/io/FileInputStream"sv
		);

	file_input_stream_fd_field_position =
		c.instance_field_position(
			u8"fd_"sv, u8"I"sv
		);

	c.declared_instance_methods().find(
		u8"read"sv, u8"()I"sv
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

	c.declared_instance_methods().find(
		u8"read"sv, u8"([BII)I"sv
	).native_function(
		(void*)+[](
			native_environment*, object* ths, object* b, int off, int len
		) -> int32 {
			handle<posix::file> fd {
				ths->get<int32>(file_input_stream_fd_field_position)
			};
			int8* data = array_data<int8>(*b);
			nuint read = fd->try_read_to(
				span{ data + off, (nuint) len },
				[](auto){ posix::abort(); }
			);
			if(read == 0) {
				return -1;
			}
			return read;
		}
	);
}