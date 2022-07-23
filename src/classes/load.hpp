#pragma once

#include "class/decl.hpp"
#include "executable_path.hpp"

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
	auto try_at = [&](auto path) -> decltype(auto) {
		auto name0 = transform_view{ name, [&](auto ch) {
			return (const char) ch;
		}};

		auto null_terminated = concat_view {
			path, single_view{'/'},
			name0, c_string{ ".class" }, single_view{ '\0' }
		};

		return view_copy_on_stack{ null_terminated }(
			[&](auto on_stack) -> optional<decltype(handler(declval<FILE*>()))> {
				FILE* f = fopen(on_stack.data(), "rb");

				if(f == nullptr) {
					return {};
				}

				on_scope_exit close_file{[&]{ fclose(f); }};

				return handler(f);
			}
		);
	};

	auto exe = executable_path.value().sized();

	// TODO replace with algo
	nuint last_slash = exe.size() - 1;

	while(
		last_slash >= 0 &&
		(exe[last_slash] != '\\' && exe[last_slash] != '/')
	){
		--last_slash;
	}

	auto result = view_copy_on_stack {
		concat_view {
			c_string{ exe.data(), last_slash },
			c_string{ "/java.base"}
		}
	}([&](auto on_stack) {
		return try_at(on_stack);
	});

	if(!result.has_value()) {
		result = try_at(c_string{ "." });
	}

	if(!result) {
		fputs("couldn't find class file ", stderr);
		view_copy_on_stack{ name }([&](auto name_on_stack) {
			fwrite(name_on_stack.data(), 1, name_on_stack.size(), stderr);
		});
		abort();
	}

	return result.value();
}

template<range Name>
inline _class& load_class(Name name);