#pragma once

#include "container.hpp"

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