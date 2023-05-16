#pragma once

#include "decl/classes.hpp"

template<basic_range Name>
c& classes::define_array_class(
	Name&& name, object_of<jl::c_loader>* defining_loader
) {
	mutex_->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		mutex_->unlock();
	}};

	posix::memory_for_range_of<instance_field> declared_instance_fields {
		posix::allocate_memory_for<instance_field>(2)
	};

	// ptr to data
	declared_instance_fields[0].construct(
		class_file::access_flags{ class_file::access_flag::_private },
		class_file::constant::utf8{ nullptr, 0 },
		class_file::constant::utf8{ c_string{ u8"J" } }
	);
	// length
	declared_instance_fields[1].construct(
		class_file::access_flags{ class_file::access_flag::_private },
		class_file::constant::utf8{ nullptr, 0 },
		class_file::constant::utf8{ c_string{ u8"I" } }
	);

	posix::memory_for_range_of<utf8::unit> descriptor
		= posix::allocate_memory_for<utf8::unit>(range_size(name));
	range{ name }.copy_to(descriptor.as_span());

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
		object_class.get(),
		posix::memory_for_range_of<c*>{},
		posix::memory_for_range_of<static_field>{},
		move(declared_instance_fields),
		posix::memory_for_range_of<static_method>{},
		posix::memory_for_range_of<instance_method>{},
		optional<method>{},
		is_array_class{ true },
		is_primitive_class{ false },
		defining_loader == nullptr ? nullptr_ref : reference{ *defining_loader }
	);
}