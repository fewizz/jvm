#pragma once

#include "./find_or_load.hpp"
#include "./load.hpp"

#include "execute.hpp"
#include "execution/info.hpp"
#include "define/class.hpp"
#include "define/primitive_class.hpp"
#include "define/array_class.hpp"

#include <core/ends_with.hpp>
#include <core/starts_with.hpp>

#include <stdio.h>

template<range Name>
static inline _class& find_or_load_class(Name&& name) {
	if(optional<_class&> c = try_find_class(name); c.has_value()) {
		return c.value();
	}
	return load_class(name);
}

// only for loading NON-LOADED classes.
// otherwise, use find_or_load_class(name)
template<range Name>
inline _class& load_class(Name&& name) {
	if(info) {
		tabs();
		fputs("loading class ", stderr);
	}

	view_copy_on_stack{ name }([&](auto on_stack) {
		fwrite(on_stack.data(), 1, on_stack.size(), stderr);
		fputc('\n', stderr);
	});

	if(starts{ name }.with('[')) {
		return define_array_class(name);
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