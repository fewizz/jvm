#include "class.hpp"
#include "method.hpp"
#include "class/constants.hpp"

#include <class_file/method/reader.hpp>
#include <class_file/descriptor/reader.hpp>

#include <elements/of.hpp>
#include <elements/one_of.hpp>

template<typename Iterator>
static inline elements::of<
	method, Iterator
>
read_method_and_get_advaned_iterator(
	constants& const_pool, class_file::method::reader<Iterator> reader
) {
	auto [access_flags, name_index_reader] {
		reader.read_access_flags_and_get_name_index_reader()
	};
	auto [name_index, descriptor_index_reader] {
		name_index_reader.read_and_get_descriptor_index_reader()
	};
	auto [desc_index, attributes_reader] {
		descriptor_index_reader.read_and_get_attributes_reader()
	};

	code_or_native_function code_or_native_function {
		optional<native_function&>()
	};

	exception_handlers exception_handlers;

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

				exception_handlers = {
					allocate_for<
						class_file::attribute::code::exception_handler
					>(read_exception_table.count())
				};

				read_exception_table([&](exception_handler eh) {
					exception_handlers.emplace_back(eh);
					return loop_action::next;
				});
			}
		}
	);

	auto descriptor_parameters_reader = class_file::descriptor::method_reader {
		const_pool.utf8_constant(desc_index).iterator()
	};

	parameters_count params_count{0};
	descriptor_parameters_reader([&](auto) {
		++params_count;
		return true;
	});

	parameters_type_names parameter_names {
		allocate_for<c_string_of_known_size>(params_count)
	};

	descriptor_parameters_reader.parameters_names(
		[&](c_string_of_known_size parameter_name) {
			parameter_names.emplace_back(parameter_name);
			return true;
		}
	);

	class_file::constant::utf8 name = const_pool.utf8_constant(name_index);
	class_file::constant::utf8 desc = const_pool.utf8_constant(desc_index);

	return {
		method {
			access_flags,
			name,
			desc,
			move(parameter_names),
			code_or_native_function,
			move(exception_handlers)
		},
		it
	};
}