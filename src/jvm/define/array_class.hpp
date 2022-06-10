#pragma once

#include "../class.hpp"

#include <core/copy.hpp>
#include <core/concat.hpp>
#include <core/c_string.hpp>

inline _class& define_array_class(_class& element_class) {
	classes.emplace_back(const_pool{ 6 });
	_class& c = classes.back();

	auto name = concat_view{ element_class.name(), array{ '[', ']' } };
	c_string ptr_name { "ptr_" };
	c_string ptr_desc { "J" };
	c_string len_name { "len_" };
	c_string len_desc { "I" };

	c.data_ = { default_allocator{}.allocate(name.size()), name.size() };

	c.const_pool::emplace_back(class_file::constant::utf8 {
		(char*) c.data_.data(), (uint16) name.size()
	});

	c.const_pool::emplace_back(class_file::constant::_class { 1 });

	c.const_pool::emplace_back(class_file::constant::utf8 {
		ptr_name.data(), (uint16) ptr_name.size() }
	);

	c.const_pool::emplace_back(class_file::constant::utf8 {
		ptr_desc.data(), (uint16) ptr_desc.size()
	});

	c.const_pool::emplace_back(class_file::constant::utf8 {
		len_name.data(), (uint16) len_name.size()
	});

	c.const_pool::emplace_back(class_file::constant::utf8 {
		len_desc.data(), (uint16) len_desc.size()
	});

	c.fields_ = { 2 };
	c.fields_.emplace_back(
		field {
			c,
			class_file::access_flags{ class_file::access_flag::_private },
			name_index{ 3 }, descriptor_index{ 4 }
		}
	);
	c.fields_.emplace_back(
		field {
			c,
			class_file::access_flags{ class_file::access_flag::_private },
			name_index{ 5 }, descriptor_index{ 6 }
		}
	);

	c.instance_fields_ = { 2 };
	c.instance_fields_.emplace_back(c.fields_[0].get<field>());
	c.instance_fields_.emplace_back(c.fields_[1].get<field>());

	c.this_class_index_ = name_index{ 2 };

	return c;
}