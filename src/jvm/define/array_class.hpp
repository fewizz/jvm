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
	static_assert(is_array<remove_reference<decltype("dsf")>>);
	auto concated = concat_view(name, ptr_name, ptr_desc, len_name, len_desc);
	auto concated_size = concated.size();

	c.data_ = { default_allocator{}.allocate(concated_size), concated_size };
	copy{ concated }.to(c.data_);

	auto ptr = (char*) c.data_.data();
	c.const_pool::emplace_back(
		class_file::constant::utf8 { ptr, (uint16) name.size() }
	);
	c.const_pool::emplace_back(class_file::constant::_class { 1 });
	ptr += name.size();

	c.const_pool::emplace_back(
		class_file::constant::utf8 { ptr, (uint16) ptr_name.size() }
	);
	ptr += ptr_name.size();

	c.const_pool::emplace_back(
		class_file::constant::utf8 { ptr, (uint16) ptr_desc.size() }
	);
	ptr += ptr_desc.size();

	c.const_pool::emplace_back(
		class_file::constant::utf8 { ptr, (uint16) len_name.size() }
	);
	ptr += len_name.size();

	c.const_pool::emplace_back(
		class_file::constant::utf8 { ptr, (uint16) len_desc.size() }
	);
	//ptr += len_desc.size();

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
	c.instance_fields_.emplace_back(&c.fields_[0].get<field>());
	c.instance_fields_.emplace_back(&c.fields_[1].get<field>());

	c.this_class_index_ = name_index{ 2 };

	return c;
}