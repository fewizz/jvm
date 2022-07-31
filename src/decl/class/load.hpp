#pragma once

#include "execution/info.hpp"
#include "view_class_file.hpp"
#include "class.hpp"
#include "define/class.hpp"

#include <core/range.hpp>
#include <core/starts_with.hpp>

#include <stdio.h>

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