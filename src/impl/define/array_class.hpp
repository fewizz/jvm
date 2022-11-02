#include "classes.hpp"
#include "class.hpp"

#include "lib/java/lang/object.hpp"

#include <array.hpp>
#include <c_string.hpp>

template<basic_range Name>
inline _class& define_array_class(Name&& name) {
	auto data = posix::allocate_memory_for<uint8>(name.size());
	span<char> data_as_span{ (char*) data.iterator(), data.size() };
	range{ name }.copy_to(data_as_span);

	declared_fields declared_fields {
		posix::allocate_memory_for<field>(2)
	};

	// ptr to data
	declared_fields[0].construct(
		class_file::access_flags{ class_file::access_flag::_private },
		class_file::constant::utf8{ nullptr, 0 },
		class_file::constant::utf8{ c_string{ "J" } }
	);
	// length
	declared_fields[1].construct(
		class_file::access_flags{ class_file::access_flag::_private },
		class_file::constant::utf8{ nullptr, 0 },
		class_file::constant::utf8{ c_string{ "I" } }
	);

	auto descriptor = posix::allocate_memory_for<uint8>(range_size(name));
	name.copy_to(span{ (char*) descriptor.iterator(), range_size(descriptor) });

	return classes.emplace_back(
		constants{}, bootstrap_methods{},
		move(data),
		class_file::access_flags{ class_file::access_flag::_public },
		this_class_name { data_as_span },
		move(descriptor),
		object_class.value(),
		declared_interfaces{posix::memory_for_range_of<_class*>{}},
		move(declared_fields),
		declared_methods{posix::memory_for_range_of<method>{}},
		is_array_class{ true },
		is_primitive_class{ false }
	);
}