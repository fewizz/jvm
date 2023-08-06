#include "decl/field.hpp"

#include <class_file/descriptor/field.hpp>
#include <posix/abort.hpp>

inline field::field(
	class_file::access_flags   access_flags,
	class_file::constant::utf8 name,
	class_file::constant::utf8 desc
) :
	class_member{ access_flags, name, desc },
	type {
		class_file::read_field_descriptor(
			desc.iterator(),
			overloaded {
				[&]<class_file::primitive_type Type>
				-> one_of_descriptor_field_types
				{
					return Type{};
				},
				[&](class_file::reference_type auto ref_type)
				-> one_of_descriptor_field_types
				{
					return ref_type;
				}
			},
			[](auto) -> one_of_descriptor_field_types {
				posix::abort();
			}
		)
	},
	stack_size {
		(uint8) ((
			type.is_same_as<class_file::j>() ||
			type.is_same_as<class_file::d>()
		) ? 2 : 1)
	}
{}