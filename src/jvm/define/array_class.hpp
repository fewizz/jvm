#pragma once

#include "../class/declaration.hpp"
#include "../classes/container.hpp"

#include <core/array.hpp>
#include <core/copy.hpp>
#include <core/concat.hpp>
#include <core/c_string.hpp>

inline _class& define_array_class(_class& element_class) {
	const_pool const_pool{ 6 };

	auto name = concat_view{ element_class.name(), array{ '[', ']' } };
	c_string ptr_name { "ptr_" };
	c_string ptr_desc { "J" };
	c_string len_name { "len_" };
	c_string len_desc { "I" };

	span data{ default_allocator{}.allocate(name.size()), name.size() };

	const_pool.emplace_back(class_file::constant::utf8 {
		(char*) data.data(), (uint16) name.size()
	});

	const_pool.emplace_back(class_file::constant::_class { 1 });

	const_pool.emplace_back(class_file::constant::utf8 {
		ptr_name.data(), (uint16) ptr_name.size() }
	);

	const_pool.emplace_back(class_file::constant::utf8 {
		ptr_desc.data(), (uint16) ptr_desc.size()
	});

	const_pool.emplace_back(class_file::constant::utf8 {
		len_name.data(), (uint16) len_name.size()
	});

	const_pool.emplace_back(class_file::constant::utf8 {
		len_desc.data(), (uint16) len_desc.size()
	});

	fields_container fields{ 2 };

	fields.emplace_back(
		field {
			class_file::access_flags{ class_file::access_flag::_private },
			name_index{ 3 }, descriptor_index{ 4 }
		}
	);
	fields.emplace_back(
		field {
			class_file::access_flags{ class_file::access_flag::_private },
			name_index{ 5 }, descriptor_index{ 6 }
		}
	);

	this_class_index this_index{ 2 };

	classes.emplace_back(
		move(const_pool), data,
		class_file::access_flags{ class_file::access_flag::_public },
		this_index, super_class_index{}, // TODO extend from Object
		interfaces_indices_container{},
		move(fields), methods_container{}
	);

	return classes.back();
}