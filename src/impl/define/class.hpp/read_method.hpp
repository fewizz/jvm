#include "class.hpp"
#include "method.hpp"
#include "class/const_pool.hpp"

#include <class_file/method/reader.hpp>
#include <class_file/descriptor/reader.hpp>

#include <core/meta/elements/of.hpp>
#include <core/meta/elements/one_of.hpp>

template<typename Iterator>
static inline elements::of<
	method, Iterator
>
read_method_and_get_advaned_iterator(
	const_pool& const_pool, class_file::method::reader<Iterator> reader
) {
	auto [access_flags, name_index_reader] {
		reader.read_access_flags_and_get_name_index_reader()
	};
	auto [name_index, descriptor_index_reader] {
		name_index_reader.read_and_get_descriptor_index_reader()
	};
	auto [descriptor_index, attributes_reader] {
		descriptor_index_reader.read_and_get_attributes_reader()
	};

	code_or_native_function code_or_native_function{ elements::none{} };

	exception_handlers_container exception_handlers;

	Iterator it = attributes_reader.read_and_get_advanced_iterator(
		[&](auto name_index) {
			return const_pool.utf8_constant(name_index);
		},
		[&]<typename Type>(Type x) {
			using namespace class_file;

			if constexpr (Type::attribute_type == attribute::type::code) {
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

	auto descriptor_parameters_reader = class_file::descriptor::method_reader {
		const_pool.utf8_constant(descriptor_index).begin()
	};

	arguments_count args_count{0};

	descriptor_parameters_reader([&](auto) {
		++args_count;
		return true;
	});

	return {
		method {
			access_flags,
			class_file::constant::name_index{ name_index },
			class_file::constant::descriptor_index{ descriptor_index },
			args_count,
			code_or_native_function,
			move(exception_handlers)
		},
		it
	};
}