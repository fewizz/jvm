#pragma once

#include "../alloc.hpp"

#include <core/limited_list.hpp>
#include <core/meta/elements/of.hpp>
#include <core/c_string.hpp>
#include <core/equals.hpp>

inline limited_list<
	elements::of<
		c_string<c_string_type::known_size, char>,
		void*
	>,
	uint16,
	default_allocator
> native_methods{ 4096 };

template<range Name>
void* try_find_native_method(Name&& name) {
	for(auto& e : native_methods) {
		if(equals(e.at<0>(), name)) {
			return e.at<1>();
		}
	}
	return nullptr;
}