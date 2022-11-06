#include "decl/array.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <posix/io.hpp>

static instance_field_position file_output_stream_fd_field_position;

static inline void init_java_io_file_output_stream() {
	_class& file_output_stream_class = classes.find_or_load(
		c_string{ "java/io/FileOutputStream" }
	);

	file_output_stream_fd_field_position =
		file_output_stream_class.instance_field_position(
			c_string{ "fd_value_" }, c_string{ "I" }
		);

	file_output_stream_class.declared_methods().find(
		c_string{ "write" }, c_string{ "(I)V" }
	).native_function(
		(void*) (void (*)(native_environment*, object*, int32))
		[](native_environment*, object* ths, int32 value) {
			posix::file_descriptor fd0 {
				ths->get<int32>(file_output_stream_fd_field_position)
			};
			bool result = true;
			fd0.try_write_from(
				array{ (char) value },
				[&]([[maybe_unused]]auto error) { result = false; }
			);
			if(!result) {
				abort();
				// thrown = create_io_exception TODO
			}
		}
	);

	file_output_stream_class.declared_methods().find(
		c_string{ "write" }, c_string{ "([B)V" }
	).native_function(
		(void*) (void (*)(native_environment*, object*, object*))
		[](native_environment*, object* ths, object* a) {
			posix::file_descriptor fd0 {
				ths->get<int32>(file_output_stream_fd_field_position)
			};
			int8* data = array_data<int8>(*a);
			bool result = true;
			int32 size = array_length(*a);
			fd0.try_write_from(
				span{ data, (nuint) size },
				[&]([[maybe_unused]]auto error) { result = false; }
			);
			if(!result) {
				abort();
			}
		}
	);

	file_output_stream_class.declared_methods().find(
		c_string{ "write" }, c_string{ "([BII)V" }
	).native_function(
		(void*) (void (*)(
			native_environment*, object*, object*, int32, int32
		))
		[](
			native_environment*, object* ths,
			object* a, int32 off, int32 len
		) {
			posix::file_descriptor fd0 {
				ths->get<int32>(file_output_stream_fd_field_position)
			};
			int8* data = array_data<int8>(*a);
			bool result = true;
			fd0.try_write_from(
				span{ data + off, (nuint) len },
				[&]([[maybe_unused]]auto error) { result = false; }
			);
			if(!result) {
				abort();
			}
		}
	);

	file_output_stream_class.declared_methods().find(
		c_string{ "close" }, c_string{ "()V" }
	).native_function(
		(void*) (void (*)(native_environment*, object*))
		[](native_environment*, object* ths) {
			posix::file_descriptor fd0 {
				ths->get<int32>(file_output_stream_fd_field_position)
			};
			bool result = true;
			posix::try_close_file(
				posix::file_descriptor{ fd0 },
				[&]([[maybe_unused]] auto error) { result = false; }
			);
			if(!result) {
				abort();
			}
		}
	);

}