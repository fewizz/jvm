#pragma once

#include "../class/declaration.hpp"
#include "../classes/container.hpp"

#include <core/range.hpp>
#include <core/copy.hpp>

template<range Name>
static inline _class& define_primitive_class(Name&& name) {
	const_pool const_pool{ 2 };

	span<uint8> data {
		default_allocator{}.allocate(name.size()), (uint16) name.size()
	};
	copy{ name }.to(data);

	const_pool.emplace_back(
		class_file::constant::utf8 {
			(char*) data.data(), (uint16) name.size()
		}
	);
	const_pool.emplace_back(class_file::constant::_class{ 1 });

	this_class_index this_index{ 2 };

	classes.emplace_back(
		move(const_pool), data,
		class_file::access_flags{ class_file::access_flag::_public },
		this_index, super_class_index{ 0 }, // TODO extend from Object?
		interfaces_indices_container{},
		instance_fields_container{},
		static_fields_container{},
		methods_container{}
	);

	return classes.back();
}