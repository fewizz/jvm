#pragma once

#include "class/decl.hpp"

#include <core/limited_list.hpp>

struct classes_container :
	limited_list<_class, uint32, default_allocator>
{
	using base_type = limited_list<_class, uint32, default_allocator>;
	using base_type::base_type;

	template<typename... Args>
	void emplace_back(Args&&... args) {
		new (base_type::ptr_ + base_type::size_)
			value_type{ forward<Args>(args)... };
		++size_;
	}

};

static inline classes_container classes{ 65536 };