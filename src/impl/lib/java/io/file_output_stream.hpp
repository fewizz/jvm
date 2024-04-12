#include "decl/array.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <posix/io.hpp>

static layout::position file_output_stream_fd_field_position;

static inline void init_java_io_file_output_stream() {
	c& file_output_stream_class
		= classes.load_class_by_bootstrap_class_loader(
			u8"java/io/FileOutputStream"s
		);

	file_output_stream_fd_field_position =
		file_output_stream_class.instance_field_position(
			u8"fd_"s, u8"I"s
		);

	file_output_stream_class.declared_instance_methods().find(
		u8"write"s, u8"(I)V"s
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
		u8"write"s, u8"([B)V"s
	).native_function(
		(void*)+[](native_environment*, object* ths, object* a) {
			handle<posix::file> fd {
				ths->get<int32>(file_output_stream_fd_field_position)
			};
			int8* data = array_data<int8>(*a);
			int32 size = array_length(*a);
			fd->try_write_from(
				span{ data, (nuint) size },
				[&](auto) { posix::abort(); }
			);
		}
	);

	file_output_stream_class.declared_instance_methods().find(
		u8"write"s, u8"([BII)V"s
	).native_function(
		(void*)+[](
			native_environment*, object* ths,
			object* a, int32 off, int32 len
		) {
			handle<posix::file> fd {
				ths->get<int32>(file_output_stream_fd_field_position)
			};
			int8* data = array_data<int8>(*a);
			fd->try_write_from(
				span{ data + off, (nuint) len },
				[&](auto) { posix::abort(); }
			);
		}
	);

	file_output_stream_class.declared_instance_methods().find(
		u8"close"s, u8"()V"s
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