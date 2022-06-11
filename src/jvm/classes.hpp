#pragma once

#include "class.hpp"

#include <core/limited_list.hpp>

static inline limited_list<_class, uint32, default_allocator> classes{ 65536 };

#include <core/equals.hpp>

template<range Name>
static inline optional<_class&> try_find_class(Name name) {
	for(auto& c : classes) {
		if(equals(c.name(), name)) return { c };
	}
	return elements::none{};
}

template<range Name>
static inline _class& find_class(Name name) {
	optional<_class&> raw = try_find_class(name);
	if(raw.has_value()) {
		fprintf(stderr, "couldn't find class");
		abort();
	}
	return raw.value();
}

#include "load.hpp"

template<range Name>
static inline _class& find_or_load(Name name) {
	if(optional<_class&> c = try_find_class(name); c.has_value()) {
		return c.value();
	}
	return load_class(name);
}

template<range Name>
static inline bool class_is_defined(Name name) {
	return try_find_class(name).has_value();
}