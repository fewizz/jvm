#pragma once

#include "executable_path.hpp"
#include "abort.hpp"

#include <range.hpp>
#include <ranges.hpp>
#include <c_string.hpp>
#include <optional.hpp>
#include <on_scope_exit.hpp>
#include <expression_of_type.hpp>

#include <stdio.h>

template<basic_range Name, typename Handler>
inline decltype(auto) view_class_file(Name&& name, Handler&& handler) {
	auto try_at = [&](auto path) -> decltype(auto) {
		auto null_terminated = ranges {
			path, array{'/'},
			name, c_string{ ".class" }, array{ '\0' }
		}.concat_view();

		return range{ null_terminated }.view_copied_elements_on_stack(
			[&](auto on_stack)
			-> optional<decltype(handler(expression_of_type<FILE*>))> {
				FILE* f = fopen(on_stack.elements_ptr(), "rb");

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

	auto result = range {
		ranges {
			c_string{ exe.elements_ptr(), last_slash },
			c_string{ "/java.base"}
		}.concat_view()
	}.view_copied_elements_on_stack([&](auto on_stack) {
		return try_at(on_stack);
	});

	if(!result.has_value()) {
		result = try_at(c_string{ "." });
	}

	if(!result) {
		fputs("couldn't find class file ", stderr);
		range{ name}.view_copied_elements_on_stack([&](auto name_on_stack) {
			fwrite(
				name_on_stack.elements_ptr(), 1, name_on_stack.size(), stderr
			);
		});
		abort();
	}

	return result.value();
}