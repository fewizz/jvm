#pragma once

#include "decl/classes.hpp"

template<basic_range Name>
c& classes::define_array_class(
	Name&& name, j::c_loader* defining_loader
) {
	mutex_->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		mutex_->unlock();
	}};

	auto declared_instance_fields {
		posix::allocate<instance_field>(2)
	};

	// ptr to data
	declared_instance_fields[0].construct(
		class_file::access_flags{ class_file::access_flag::_private },
		class_file::constant::utf8{ nullptr, 0 },
		class_file::constant::utf8{ u8"J" }
	);
	// length
	declared_instance_fields[1].construct(
		class_file::access_flags{ class_file::access_flag::_private },
		class_file::constant::utf8{ nullptr, 0 },
		class_file::constant::utf8{ u8"I" }
	);

	class_data_t data{};

	data.emplace_back(posix::allocate(range_size(name)));

	range{ name }.copy_to(data.back().as_span().cast<utf8::unit>());

	class_file::constant::utf8 this_name = data.back().cast<utf8::unit>();

	class_file::constant::utf8 descriptor = this_name;

	return emplace_back(
		constants{},
		bootstrap_methods{},
		move(data),
		class_file::access_flags{ class_file::access_flag::_public },
		this_name,
		descriptor,
		class_file::constant::utf8{},
		object_class.get(),
		posix::memory<c*>{},
		posix::memory<static_field>{},
		move(declared_instance_fields),
		posix::memory<static_method>{},
		posix::memory<instance_method>{},
		optional<method>{},
		is_array_class{ true },
		is_primitive_class{ false },
		defining_loader == nullptr ? nullptr_ref : reference{ *defining_loader }
	);
}