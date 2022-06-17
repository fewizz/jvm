#pragma once

#include "../class/declaration.hpp"

#include <core/range.hpp>
#include <core/transform.hpp>
#include <core/view_copy_on_stack.hpp>
#include <core/single.hpp>
#include <core/concat.hpp>
#include <core/c_string.hpp>
#include <core/on_scope_exit.hpp>

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
inline _class& load_class(Name name);