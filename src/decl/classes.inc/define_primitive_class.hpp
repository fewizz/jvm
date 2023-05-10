#pragma once

#include "decl/classes.hpp"

template<basic_range Name>
c& classes::define_primitive_class(Name&& name, char ch) {
	// should be called at the beginning, no lock needed

	posix::memory_for_range_of<utf8::unit> descriptor
		= posix::allocate_memory_for<utf8::unit>(1);

	descriptor[0].construct((uint8)ch);

	posix::memory_for_range_of<uint8> data
		= posix::allocate_memory_for<uint8>(name.size());
	
	range{ name }.copy_to(data.as_span<utf8::unit>());

	class_file::constant::utf8 this_name = data.as_span<utf8::unit>();

	return emplace_back(
		constants{}, bootstrap_methods{},
		move(data),
		class_file::access_flags{ class_file::access_flag::_public },
		this_name,
		move(descriptor),
		class_file::constant::utf8{},
		object_class,
		posix::memory_for_range_of<c*>{},
		posix::memory_for_range_of<static_field>{},
		posix::memory_for_range_of<instance_field>{},
		posix::memory_for_range_of<static_method>{},
		posix::memory_for_range_of<instance_method>{},
		optional<method>{},
		is_array_class{ false },
		is_primitive_class{ true }
	);
}