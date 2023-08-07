#include "decl/array.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <posix/io.hpp>

static layout::position file_output_stream_fd_field_position;

static inline void init_java_io_file_output_stream() {
	c& file_output_stream_class
		= classes.load_class_by_bootstrap_class_loader(
			c_string{ u8"java/io/FileOutputStream" }
		);

	file_output_stream_fd_field_position =
		file_output_stream_class.instance_field_position(
			c_string{ u8"fd_" }, c_string{ u8"I" }
		);

	file_output_stream_class.declared_instance_methods().find(
		c_string{ u8"write" }, c_string{ u8"(I)V" }
	).native_function(
		(void*)+[](native_environment*, object* ths, int32 value) {
			handle<posix::file> fd {
				ths->get<int32>(file_output_stream_fd_field_position)
			};
			bool result = true;
			fd->try_write_from(
				array{ (char) value },
				[&]([[maybe_unused]]auto error) { result = false; }
			);
			if(!result) {
				posix::abort();
				// thrown = create_io_exception TODO
			}
		}
	);

	file_output_stream_class.declared_instance_methods().find(
		c_string{ u8"write" }, c_string{ u8"([B)V" }
	).native_function(
		(void*)+[](native_environment*, object* ths, object* a) {
			handle<posix::file> fd {
				ths->get<int32>(file_output_stream_fd_field_position)
			};
			int8* data = array_data<int8>(*a);
			bool result = true;
			int32 size = array_length(*a);
			fd->try_write_from(
				span{ data, (nuint) size },
				[&]([[maybe_unused]]auto error) { result = false; }
			);
			if(!result) {
				posix::abort();
			}
		}
	);

	file_output_stream_class.declared_instance_methods().find(
		c_string{ u8"write" }, c_string{ u8"([BII)V" }
	).native_function(
		(void*)+[](
			native_environment*, object* ths,
			object* a, int32 off, int32 len
		) {
			handle<posix::file> fd {
				ths->get<int32>(file_output_stream_fd_field_position)
			};
			int8* data = array_data<int8>(*a);
			bool result = true;
			fd->try_write_from(
				span{ data + off, (nuint) len },
				[&]([[maybe_unused]]auto error) { result = false; }
			);
			if(!result) {
				posix::abort();
			}
		}
	);

	file_output_stream_class.declared_instance_methods().find(
		c_string{ u8"close" }, c_string{ u8"()V" }
	).native_function(
		(void*)+[](native_environment*, object* ths) {
			handle<posix::file> fd {
				ths->get<int32>(file_output_stream_fd_field_position)
			};
			bool result = true;
			posix::try_close(
				fd,
				[&]([[maybe_unused]] auto error) { result = false; }
			);
			if(!result) {
				posix::abort();
			}
		}
	);

}