#pragma once

#include "class.hpp"

#include <core/range.hpp>

template<range Name>
inline _class& load_class(Name name);

#include "define.hpp"
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

#include <stdio.h>

template<range Name>
inline _class& load_class(Name name) {
	fputs("loading class ", stderr);

	view_copy_on_stack{ name }([&](auto on_stack) {
		fwrite(on_stack.data(), 1, on_stack.size(), stderr);
		fputc('\n', stderr);
	});

	if(
		equals(name, c_string{ "void"    }) ||
		equals(name, c_string{ "boolean" }) ||
		equals(name, c_string{ "byte"    }) ||
		equals(name, c_string{ "short"   }) ||
		equals(name, c_string{ "char"    }) ||
		equals(name, c_string{ "int"     }) ||
		equals(name, c_string{ "long"    }) ||
		equals(name, c_string{ "float"   }) ||
		equals(name, c_string{ "double"  })
	) {
		return define_primitive_class(forward<Name>(name));
	}

	if(ends{ name }.with(array{ '[', ']' })) {
		auto element_name {
			 to_range(name.begin(), name.begin() + (name.size() - 2))
		};
		_class& c = find_or_load(element_name);
		return define_array_class(c);
	}

	auto name0 = transform_view{ name, [&](auto ch) {
		return (const char) ch;
	}};

	auto null_terminated = concat_view {
		name0,
		c_string{ ".class" },
		array{ '\0' }
	};

	uint8* data; nuint size;

	view_copy_on_stack{ null_terminated }([&](auto on_stack) {
		FILE* f = fopen(on_stack.data(), "rb");
		on_scope_exit close_file{[&]{ fclose(f); }};
		if(f == nullptr) {
			fprintf(stderr, "couldn't open class file %s", on_stack.data());
			abort();
		}

		fseek(f, 0, SEEK_END);
		size = ftell(f);
		rewind(f);
		data = (uint8*) malloc(size);
		fread(data, 1, size, f);
	});

	auto& c = define_class(span{ data, size});

	if(
		auto clinit = c.try_find_method(c_string{ "<clinit>" });
		clinit != nullptr
	) {
		execute(*clinit);
	}

	return c;
}