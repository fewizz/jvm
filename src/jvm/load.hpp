#pragma once

#include "class.hpp"

#include <core/range.hpp>

template<range Name>
inline _class& load_class(Name name);

#include "define/class.hpp"
#include "define/primitive_class.hpp"
#include "define/array_class.hpp"
#include "execute.hpp"
#include "classes.hpp"

#include <core/range.hpp>
#include <core/transform.hpp>
#include <core/view_copy_on_stack.hpp>
#include <core/single.hpp>
#include <core/concat.hpp>
#include <core/c_string.hpp>
#include <core/on_scope_exit.hpp>
#include <core/ends_with.hpp>
#include <core/single.hpp>

#include <stdio.h>

template<typename Name, typename Handler>
inline decltype(auto) view_class_file(Name&& name, Handler&& handler) {
	auto name0 = transform_view{ name, [&](auto ch) {
		return (const char) ch;
	}};

	auto null_terminated = concat_view {
		name0, c_string{ ".class" }, single_view{ '\0' }
	};

	return view_copy_on_stack{ null_terminated }(
		[&](auto on_stack) -> decltype(auto) {
			FILE* f = fopen(on_stack.data(), "rb");
			on_scope_exit close_file{[&]{ fclose(f); }};

			if(f == nullptr) {
				fprintf(stderr, "couldn't open class file %s", on_stack.data());
				abort();
			}

			return handler(f);
		}
	);
}

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
		_class& c = find_or_load(element_name);
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