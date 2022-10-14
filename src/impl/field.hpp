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
			optional<one_of_non_void_descriptor_types> type;
			class_file::read_type_descriptor(
				desc.iterator(),
				[&]<typename Type>(Type t) {
					if constexpr(same_as<Type, class_file::v>) {
						abort();
					}
					else {
						type = t;
					}
				}, [](auto){ abort(); }
			);
			return type.value();
		}()
	},
	stack_size {
		(uint8) (type.is<class_file::j>() || type.is<class_file::d>() ? 2 : 1)
	}
{}