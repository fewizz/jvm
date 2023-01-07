#include "decl/field.hpp"

#include <class_file/descriptor/read_type.hpp>
#include <posix/abort.hpp>

inline field::field(
	class_file::access_flags   access_flags,
	class_file::constant::utf8 name,
	class_file::constant::utf8 desc
) :
	class_member{ access_flags, name, desc },
	type {
		[&] {
			optional<one_of_descriptor_parameter_types> type;
			class_file::read_field_type_descriptor(
				desc.iterator(),
				[&]<typename Type>(Type t) {
					type = t;
				}, [](auto){ posix::abort(); }
			);
			return type.get();
		}()
	},
	stack_size {
		(uint8) ((
			type.is_same_as<class_file::j>() ||
			type.is_same_as<class_file::d>()
		) ? 2 : 1)
	}
{}