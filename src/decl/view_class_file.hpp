#pragma once

#include "executable_path.hpp"

#include <array.hpp>
#include <range.hpp>
#include <ranges.hpp>
#include <c_string.hpp>
#include <optional.hpp>
#include <on_scope_exit.hpp>
#include <expression_of_type.hpp>

#include <posix/io.hpp>

template<basic_range Name, typename Handler>
inline decltype(auto) view_class_file(Name&& name, Handler&& handler) {
	auto try_at = [&](auto path) -> decltype(auto) {
		auto null_terminated = ranges {
			path, array{'/'},
			name, c_string{ ".class" }, array{ '\0' }
		}.concat_view();

		return null_terminated.view_copied_elements_on_stack(
			[&](span<const char> on_stack)
			-> optional<decltype(
				expression_of_type<Handler>
				(move(expression_of_type<body<posix::file>>))
			)> {
				bool error = false;
				optional<body<posix::file>> f = posix::try_open_file(
					c_string{ on_stack.iterator() },
					posix::file_access_modes {
						posix::file_access_mode::read,
						posix::file_access_mode::binary
					},
					[&](auto) { error = true; }
				);
				if(error) {
					return {};
				}
				return handler(move(f.get()));
			}
		);
	};

	auto exe = executable_path.get().sized();

	// TODO replace with algo
	nuint last_slash = exe.size() - 1;

	while(
		last_slash >= 0 &&
		(exe[last_slash] != '\\' && exe[last_slash] != '/')
	){
		--last_slash;
	}

	auto result =
		ranges {
			c_string{ exe.iterator(), last_slash },
			c_string{ "/java.base"}
		}
		.concat_view()
		.sized_view()
		.view_copied_elements_on_stack([&](auto on_stack) {
			return try_at(on_stack);
		});

	if(!result.has_value()) {
		result = try_at(c_string{ "." });
	}

	if(!result) {
		posix::std_err.write_from(c_string{"couldn't find class file "});
		range{ name }.view_copied_elements_on_stack([&](auto name_on_stack) {
			posix::std_err.write_from(name_on_stack);
		});
		posix::std_err.write_from(c_string{ "\n" });
		abort();
	}

	return result.get();
}