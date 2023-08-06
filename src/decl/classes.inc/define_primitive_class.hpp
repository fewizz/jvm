#pragma once

#include "decl/classes.hpp"

c& classes::define_primitive_class(
	class_file::constant::utf8 name,
	const utf8::unit& ch
) {
	// should be called at the beginning, no lock needed

	class_file::constant::utf8 descriptor{ &ch, 1 };

	return emplace_back(
		constants{}, bootstrap_methods{},
		class_data{},
		class_file::access_flags{ class_file::access_flag::_public },
		name,
		descriptor,
		class_file::constant::utf8{},
		object_class,
		posix::memory<c*>{},
		posix::memory<static_field>{},
		posix::memory<instance_field>{},
		posix::memory<static_method>{},
		posix::memory<instance_method>{},
		optional<method>{},
		is_array_class{ false },
		is_primitive_class{ true }
	);
}