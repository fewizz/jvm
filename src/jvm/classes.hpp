#pragma once

#include "class.hpp"

#include <core/limited_list.hpp>

inline limited_list<_class, uint32, default_allocator> classes{ 65536 };

#include <core/equals.hpp>

template<range Name>
_class* try_find_class(Name name) {
	for(auto& c : classes) {
		if(equals(c.name(), name)) return &c;
	}
	return nullptr;
}

template<range Name>
_class& find_class(Name name) {
	auto raw = try_find_class(name);
	if(raw == nullptr) {
		fprintf(stderr, "couldn't find class");
		abort();
	}
	return *raw;
}

#include "load.hpp"

template<range Name>
_class& find_or_load(Name name) {
	if(auto c = try_find_class(name); c != nullptr) {
		return *c;
	}
	return load_class(name);
}

template<range Name>
bool class_is_defined(Name name) {
	return try_find_class(name) != nullptr;
}