#pragma once

#include "class.hpp"
#include "class/load.hpp"

#include <memory_list.hpp>

static struct class_container :
	memory_list<_class, uint32>
{
	using base_type = memory_list<_class, uint32>;
	using base_type::base_type;

	template<basic_range Name>
	_class& find_or_load(Name&& name) {
		if(optional<_class&> c = try_find(name); c.has_value()) {
			return c.value();
		}
		return ::load_class(name);
	}

	template<basic_range Name>
	optional<_class&> try_find(Name name) {
		for(auto& c : *this) {
			if(equals(c.name(), name)) {
				return { c };
			}
		}
		return elements::none{};
	}

	template<basic_range Name>
	_class& find_class(Name name) {
		optional<_class&> raw = try_find(name);
		if(!raw.has_value()) {
			fprintf(stderr, "couldn't find class");
			abort();
		}
		return raw.value();
	}

	template<basic_range Name>
	static inline bool contains(Name name) {
		return try_find_class(name).has_value();
	}

} classes{ allocate(sizeof(_class) * 65536) };