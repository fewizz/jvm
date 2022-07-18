#pragma once

#include "../class/decl.hpp"
#include "../method/decl.hpp"
#include "class/file/method/reader.hpp"

#include <core/meta/elements/of.hpp>
#include <core/meta/elements/one_of.hpp>

template<typename Iterator>
static inline elements::of<
	class_file::method::reader<Iterator, class_file::method::reader_stage::end>,
	method
>
read_method(
	const_pool& const_pool, class_file::method::reader<Iterator> read_access
) {
	auto [read_name_index, access_flags] = read_access();
	auto [descriptor_reader, name_index] = read_name_index();
	auto [read_attributes, descriptor_index] = descriptor_reader();

	code_or_native_function code_or_native_function{ elements::none{} };

	exception_handlers_container exception_handlers;

	auto end = read_attributes(
		[&](auto name_index) {
			return const_pool.utf8_constant(name_index);
		},
		[&]<typename Type>(Type x) {
			using namespace class_file;

			if constexpr (Type::type == attribute::type::code) {
				using namespace attribute::code;

				auto [read_max_locals, max_stack] = x();
				auto [read_code, max_locals] = read_max_locals();

				auto [read_exception_table, code_span] = read_code.as_span();

				code_or_native_function = ::code {
					code_span, max_stack, max_locals
				};

				exception_handlers = { read_exception_table.count() };

				read_exception_table([&](exception_handler eh) {
					exception_handlers.emplace_back(eh);
					return loop_action::next;
				});
			}
		}
	);

	return {
		end,
		method {
			access_flags,
			::name_index{ name_index },
			::descriptor_index{ descriptor_index },
			code_or_native_function,
			move(exception_handlers)
		}
	};
}