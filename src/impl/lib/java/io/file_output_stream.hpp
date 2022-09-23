#include "decl/array.hpp"
#include "decl/classes.hpp"
#include "decl/native/interface/environment.hpp"

#include <posix/io.hpp>

static inline void init_java_io_file_output_stream() {
	_class& file_output_stream_class = classes.find_or_load(
		c_string{ "java/io/FileOutputStream" }
	);

	file_output_stream_class.declared_methods().find(
		c_string{ "write" }, c_string{ "(JI)Z" }
	).native_function(
		(void*) (bool (*)(native_interface_environment*, int64, int32))
		[](native_interface_environment*, int64 fd, int32 value) {
			posix::file_descriptor fd0{ (int32)fd };
			bool result = true;
			fd0.try_write_from(
				array{ (char) value },
				[&]([[maybe_unused]]auto error) { result = false; }
			);
			return result;
		}
	);

	file_output_stream_class.declared_methods().find(
		c_string{ "write_buffer" }, c_string{ "(J[BII)Z" }
	).native_function(
		(void*) (bool (*)(native_interface_environment*, int64, object*, int32, int32))
		[](native_interface_environment*, int64 fd, object* a, int32 o, int32 l) {
			int8* data = array_data<int8>(*a);
			posix::file_descriptor fd0{ (int32)fd };
			bool result = true;
			fd0.try_write_from(
				span{ data + o, (nuint) l },
				[&]([[maybe_unused]]auto error) { result = false; }
			);
			return result;
		}
	);

	file_output_stream_class.declared_methods().find(
		c_string{ "close" }, c_string{ "(J)Z" }
	).native_function(
		(void*) (bool (*)(native_interface_environment*, int64))
		[](native_interface_environment*, int64 fd) {
			bool result = true;
			posix::try_close_file(
				posix::file_descriptor{ (int32) fd },
				[&]([[maybe_unused]] auto error) { result = false; }
			);
			return result;
		}
	);

}