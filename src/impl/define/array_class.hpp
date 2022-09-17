#include "classes.hpp"
#include "class.hpp"

#include "lib/java/lang/object.hpp"

#include <array.hpp>
#include <c_string.hpp>

template<basic_range Name>
inline _class& define_array_class(Name&& name) {
	auto data = posix::allocate_memory_for<uint8>(name.size());
	range{ name }.copy_to(data);

	declared_fields declared_fields {
		posix::allocate_memory_for<field>(2)
	};

	// ptr to data
	declared_fields.emplace_back(
		class_file::access_flags{ class_file::access_flag::_private },
		class_file::constant::utf8{ nullptr, 0 },
		class_file::constant::utf8{ c_string{ "J" } }
	);
	// length
	declared_fields.emplace_back(
		class_file::access_flags{ class_file::access_flag::_private },
		class_file::constant::utf8{ nullptr, 0 },
		class_file::constant::utf8{ c_string{ "I" } }
	);

	return classes.emplace_back(
		constants{}, bootstrap_methods{},
		data,
		class_file::access_flags{ class_file::access_flag::_public },
		this_class_name { move(data) },
		object_class.value(),
		declared_interfaces{},
		move(declared_fields),
		declared_methods{},
		is_array_class{ true },
		is_primitive_class{ false }
	);
}