#pragma once

#include "../class/declaration.hpp"
#include "../classes/container.hpp"

#include <core/range.hpp>
#include <core/copy.hpp>

template<range Name>
static inline _class& define_primitive_class(Name&& name) {
	classes.emplace_back(const_pool{ 2 });
	_class& c = classes.back();

	c.data_ = {
		default_allocator{}.allocate(name.size()), (uint16) name.size()
	};
	copy{ name }.to(c.data_);

	c.const_pool::emplace_back(
		class_file::constant::utf8 {
			(char*) c.data_.data(), (uint16) name.size()
		}
	);
	c.const_pool::emplace_back(class_file::constant::_class{ 1 });

	c.this_class_index_ = name_index{ 2 };

	return c;
}