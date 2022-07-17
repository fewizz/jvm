#pragma once

#include "container.hpp"
#include "../function/decl.hpp"
#include <core/meta/elements/optional.hpp>
#include <core/range.hpp>
#include <core/equals.hpp>

template<range Name>
optional<native_function&> try_find_native_function(Name name) {
	for(native_function& f : native_functions) {
		if(equals(f.name(), name)) {
			return { f };
		}
	}
	return elements::none{};
}

#include "../../../abort.hpp"
#include <stdio.h>

template<range Name>
native_function& find_native_function(Name name) {
	if(
		optional<native_function&> f {
			try_find_native_function(forward<Name>(name))
		};
		f.has_value()
	) {
		return f.value();
	}
	fputs("couldn't find native function in container", stderr);
	abort();
}

#include "../../method/decl.hpp"
#include "../../class/decl.hpp"
#include <core/concat.hpp>
#include <core/transform.hpp>

inline native_function& find_native_function(method_with_class m) {
	transform_view undescored_class_name{ m._class().name(), [](const char c) {
		if(c == '/') return '_';
		return c;
	} };
	concat_view name {
		c_string{ "Java_" },
		undescored_class_name,
		c_string{ "_" },
		m.name()
	};

	return find_native_function(name);
}