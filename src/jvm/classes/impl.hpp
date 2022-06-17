#pragma once

#include "find_or_load.hpp"
#include "load.hpp"

template<range Name>
static inline _class& find_or_load_class(Name name) {
	if(optional<_class&> c = try_find_class(name); c.has_value()) {
		return c.value();
	}
	return load_class(name);
}

#include "../define/class.hpp"
#include "../define/primitive_class.hpp"
#include "../define/array_class.hpp"
#include "../execute.hpp"

#include <core/ends_with.hpp>

template<range Name>
inline _class& load_class(Name name) {
	fputs("loading class ", stderr);

	view_copy_on_stack{ name }([&](auto on_stack) {
		fwrite(on_stack.data(), 1, on_stack.size(), stderr);
		fputc('\n', stderr);
	});

	if(ends{ name }.with(array{ '[', ']' })) {
		auto element_name {
			 to_range(name.begin(), name.begin() + (name.size() - 2))
		};
		_class& c = find_or_load_class(element_name);
		return define_array_class(c);
	}

	return view_class_file(name,
		[&](auto f) -> decltype(auto) {
			fseek(f, 0, SEEK_END);
			nuint size = ftell(f);
			rewind(f);
			uint8* data = (uint8*) malloc(size);
			fread(data, 1, size, f);
			return define_class(span{ data, size });
		}
	);
}