#pragma once

#include "class.hpp"
#include "class/es/container.hpp"

#include <core/array.hpp>
#include <core/copy.hpp>
#include <core/concat.hpp>
#include <core/c_string.hpp>

template<range Name>
inline _class& define_array_class(Name&& name) {
	const_pool const_pool{ 9 };

	c_string ptr_name { "ptr_" };
	c_string ptr_desc { "J" };
	c_string len_name { "len_" };
	c_string len_desc { "I" };
	c_string base_class { "java/lang/Object" };

	span data {
		default_allocator{}.allocate(name.size()), nuint{ name.size() }
	};

	copy{ name }.to(data);

	// 1
	const_pool.emplace_back(class_file::constant::utf8 {
		(char*) data.data(), (uint16) name.size()
	});

	// 2
	const_pool.emplace_back(
		class_file::constant::_class { class_file::utf8_index{ 1 }}
	);

	// 3
	const_pool.emplace_back(class_file::constant::utf8 {
		ptr_name.data(), (uint16) ptr_name.size() }
	);

	// 4
	const_pool.emplace_back(class_file::constant::utf8 {
		ptr_desc.data(), (uint16) ptr_desc.size()
	});

	// 5
	const_pool.emplace_back(class_file::constant::utf8 {
		len_name.data(), (uint16) len_name.size()
	});

	// 6
	const_pool.emplace_back(class_file::constant::utf8 {
		len_desc.data(), (uint16) len_desc.size()
	});

	// 7
	const_pool.emplace_back(class_file::constant::utf8 {
		len_desc.data(), (uint16) len_desc.size()
	});

	// 8
	const_pool.emplace_back(class_file::constant::_class {
		class_file::utf8_index{ 9 }
	});

	// 9
	const_pool.emplace_back(class_file::constant::utf8 {
		base_class.data(), (uint16) base_class.size()
	});

	instance_fields_container instance_fields { 2 };

	instance_fields.emplace_back(
		class_file::access_flags{ class_file::access_flag::_private },
		name_index{ 3 }, descriptor_index{ 4 }
	);
	instance_fields.emplace_back(
		class_file::access_flags{ class_file::access_flag::_private },
		name_index{ 5 }, descriptor_index{ 6 }
	);

	return classes.emplace_back(
		move(const_pool), data,
		class_file::access_flags{ class_file::access_flag::_public },
		this_class_index{ 2 }, super_class_index{ 8 },
		interfaces_indices_container{},
		move(instance_fields),
		static_fields_container{},
		methods_container{},
		is_array_class{ true },
		is_primitive_class{ false }
	);
}