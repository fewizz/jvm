#pragma once

#include "class.hpp"
#include "class/load.hpp"

#include <core/limited_list.hpp>

static struct class_container :
	limited_list<_class, uint32, default_allocator>
{
	using base_type = limited_list<_class, uint32, default_allocator>;
	using base_type::base_type;

	template<typename... Args>
	_class& emplace_back(Args&&... args) {
		new (base_type::ptr_ + base_type::size_)
			value_type{ forward<Args>(args)... };
		_class& c = (*this)[base_type::size_];
		++size_;
		return c;
	}

	template<range Name>
	_class& find_or_load(Name&& name) {
		if(optional<_class&> c = try_find(name); c.has_value()) {
			return c.value();
		}
		return ::load_class(name);
	}

	template<range Name>
	optional<_class&> try_find(Name name) {
		for(auto& c : *this) {
			if(equals(c.name(), name)) {
				return { c };
			}
		}
		return elements::none{};
	}

	template<range Name>
	_class& find_class(Name name) {
		optional<_class&> raw = try_find(name);
		if(!raw.has_value()) {
			fprintf(stderr, "couldn't find class");
			abort();
		}
		return raw.value();
	}

	template<range Name>
	static inline bool contains(Name name) {
		return try_find_class(name).has_value();
	}

} classes{ 65536 };