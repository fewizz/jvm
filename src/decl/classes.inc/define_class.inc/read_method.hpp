#pragma once

#include "decl/method.hpp"
#include "decl/class/constants.hpp"

#include <class_file/method_reader.hpp>

template<typename Iterator>
tuple<method, Iterator> read_method(
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

	code_or_native_function_ptr code_or_native_function {
		native_function_ptr{nullptr}
	};

	posix::memory<
		class_file::attribute::code::exception_handler
	> exception_handlers{};

	posix::memory<
		tuple<uint16, class_file::line_number>
	> line_numbers{};

	auto mapper = [&](auto name_index) {
		return const_pool.utf8_constant(name_index);
	};

	Iterator it = attributes_reader.read_and_get_advanced_iterator(
		mapper, [&]<typename Type>(Type reader) {

		using namespace class_file;

		if constexpr (Type::attribute_type == attribute::type::code) {
			using namespace attribute::code;
			Type max_stack_reader = reader;

			auto [max_stack, max_locals_reader]
				= max_stack_reader.read_and_get_max_locals_reader();
			auto [max_locals, code_reader]
				= max_locals_reader.read_and_get_code_reader();

			auto code_span = code_reader.read_as_span();
			auto exception_table_reader
				= code_reader.skip_and_get_exception_table_reader();

			code_or_native_function = ::code {
				code_span, max_stack, max_locals
			};

			::list exception_handlers_list {
				posix::allocate<
					class_file::attribute::code::exception_handler
				>(exception_table_reader.read_count())
			};

			auto attributes_reader
				= exception_table_reader.read_and_get_attributes_reader(
				[&](exception_handler eh) {
					exception_handlers_list.emplace_back(eh);
					return loop_action::next;
				}
			);

			attributes_reader.read_and_get_advanced_iterator(
				mapper,
				[&]<typename CodeAttributeType>(
					CodeAttributeType reader
				) {
					using namespace class_file;

					if constexpr (
						CodeAttributeType::attribute_type ==
						attribute::type::line_numbers
					) {
						CodeAttributeType count_reader = reader;
						auto [count, line_numbers_reader]
							= count_reader
							.read_and_get_line_numbers_reader();
						
						::list line_numbers_list {
							posix::allocate<
								tuple<uint16, class_file::line_number>
							>(count)
						};

						line_numbers_reader.read_and_get_advanced_iterator(
							count,
							[&](
								uint16 start_pc,
								class_file::line_number ln
							) {
								line_numbers_list.emplace_back(
									start_pc,
									ln
								);
							}
						);
						line_numbers
							= move(line_numbers_list.storage_range());
					}
				}
			);

			exception_handlers = move(exception_handlers_list.storage_range());
		}
	});

	class_file::constant::utf8 name = const_pool.utf8_constant(name_index);
	class_file::constant::utf8 desc = const_pool.utf8_constant(desc_index);

	return {
		method {
			access_flags,
			name,
			desc,
			code_or_native_function,
			move(exception_handlers),
			move(line_numbers)
		},
		it
	};
};