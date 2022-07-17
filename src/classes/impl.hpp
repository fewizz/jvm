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

#include "../execute/info.hpp"
#include "../define/class.hpp"
#include "../define/primitive_class.hpp"
#include "../define/array_class.hpp"
#include "../execute/decl.hpp"

#include <core/ends_with.hpp>
#include <core/starts_with.hpp>

template<range Name>
inline _class& load_class(Name name) {
	if(info) {
		tabs();
		fputs("loading class ", stderr);
	}

	view_copy_on_stack{ name }([&](auto on_stack) {
		fwrite(on_stack.data(), 1, on_stack.size(), stderr);
		fputc('\n', stderr);
	});

	if(starts{ name }.with('[')) {
		if(name.size() == 2) { // primitive
			return find_class(name);
		}

		if(
			!starts{ name }.with('[', 'L') ||
			!ends  { name }.with(';')
		) {
			fputs("reference array type name should end with ';'", stderr);
			abort();
		}

		auto element_name = to_range(
			name.begin() + 2,
			name.begin() + (name.size() - 1)
		);

		find_or_load_class(element_name);
		auto name = concat_view{
			array{ '[', 'L' }, element_name, array{ ';' }
		};
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