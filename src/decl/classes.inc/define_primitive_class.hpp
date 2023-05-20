#pragma once

#include "decl/classes.hpp"

template<basic_range Name>
c& classes::define_primitive_class(Name&& name, char ch) {
	// should be called at the beginning, no lock needed

	auto descriptor = posix::allocate<utf8::unit>(1);

	descriptor[0].construct((uint8)ch);

	posix::memory<> data = posix::allocate<>(name.size());
	
	range{ name }.copy_to(data.as_span());

	class_file::constant::utf8 this_name = data.as_span().cast<utf8::unit>();

	return emplace_back(
		constants{}, bootstrap_methods{},
		move(data),
		class_file::access_flags{ class_file::access_flag::_public },
		this_name,
		move(descriptor),
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