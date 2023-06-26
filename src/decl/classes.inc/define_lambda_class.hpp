#pragma once

#include "decl/blocky_memory.hpp"
#include "decl/lib/java/lang/class_loader.hpp"

#include <range.hpp>
#include <ranges.hpp>

#include <posix/memory.hpp>

#include <class_file/attribute/code/writer.hpp>

template<basic_range Name, basic_range Descriptor>
expected<c&, reference> try_define_lamda_class(
	Name&& this_class_name,
	c& interface_to_implement,
	Descriptor&& ctor_descriptor,
	j::c_loader* defining_loader // L
) {
	uint8 parameters_count;
	list<posix::memory<one_of_descriptor_parameter_types>> parameter_types_list;

	{
		class_file::method_descriptor::reader reader {
			ctor_descriptor.iterator()
		};
		parameters_count = reader.try_read_parameters_count(
			[]([[maybe_unused]] auto err) { posix::abort(); }
		).get();

		parameter_types_list =
			posix::allocate<one_of_descriptor_parameter_types>(
				parameters_count
			);

		reader.try_read_parameter_types_and_get_return_type_reader(
			[&]<typename ParamType>(ParamType parameter_type) {
				parameter_types_list.emplace_back(parameter_type);
			},
			[](auto) { posix::abort(); }
		);
	}
	
	class_data_t data{};

	auto concat_utf8 = [&]<range_of<utf8::unit&> Range>(Range&& r)
	-> class_file::constant::utf8 {
		data.emplace_back(posix::allocate(range_size(r)));
		span s = data.back().as_span().cast<utf8::unit>();
		range{ r }.copy_to(s);
		return { s };
	};

	nuint constants_count =
		/* 0 */ 1 + // this class name
		/* 1 */ 1 + // "java/lang/Object" - super class name
		/* 2 */ 1 + // super class
		/* 3 */ 1 + // "<init>" - constructor name
		/* 4 */ 1 + // "()V" - super class constructor descriptor
		/* 5 */ 1 + // "()V" - super class constructor name and type
		/* 6 */ 1 + // method_reference to super class constructor
		/* 7 */ 1 + // constructor descriptor
		parameters_count * 3; // field's name, descriptor, nat

	list consts { posix::allocate<constant>(constants_count) };
	
	consts.emplace_back(
		concat_utf8(this_class_name)
	); /* 0 */

	consts.emplace_back(class_file::constant::utf8 {
		u8"java/lang/Object"
	}); /* 1 */
	class_file::constant::name_index super_class_name_index{ 1 };

	consts.emplace_back(class_file::constant::_class {
		.name_index = super_class_name_index
	}); /* 2 */
	class_file::constant::class_index super_class_index{ 2 };

	consts.emplace_back(class_file::constant::utf8 {
		u8"<init>"
	}); /* 3 */
	class_file::constant::name_index ctor_name_index{ 3 };

	consts.emplace_back(class_file::constant::utf8 {
		u8"()V"
	}); /* 4 */
	class_file::constant::descriptor_index super_class_ctor_desc_index{ 4 };

	consts.emplace_back(class_file::constant::name_and_type {
		.name_index = ctor_name_index,
		.descriptor_index = super_class_ctor_desc_index
	}); /* 5 */
	class_file::constant::name_and_type_index super_class_ctor_nat_index{ 5 };

	consts.emplace_back(class_file::constant::method_ref {
		.class_index = super_class_index,
		.name_and_type_index = super_class_ctor_nat_index
	}); /* 6 */
	class_file::constant::method_ref_index super_class_ctor_ref_index{ 6 };

	class_file::constant::utf8 ctor_desc_const
		= concat_utf8(ctor_descriptor);
	consts.emplace_back(
		ctor_desc_const
	); /* 7 */

	list<posix::memory<>> constructor_code = posix::allocate<uint8>(1024);
	{
		auto os = constructor_code.output_stream();
		uint8* begin = constructor_code.iterator();

		using namespace class_file::attribute::code;

		// load *this*
		instruction::write(
			os,
			instruction::a_load_0{},
			begin
		);
		instruction::write(
			os,
			instruction::invoke_special { super_class_ctor_ref_index },
			begin
		);

		parameter_types_list.for_each_indexed([&](
			auto param, nuint param_index
		) { param.view_type([&]<typename Type>() {
			class_file::constant::name_index name_index {
				(uint16) consts.size()
			};
			class_file::constant::utf8 name = concat_utf8(
				ranges {
					c_string{ "field_" },
					array{ '0' + param_index }
				}.concat_view()
			);

			class_file::constant::descriptor_index descriptor_index {
				(uint16) consts.size()
			};
			class_file::constant::utf8 descriptor = concat_utf8(
				param.utf8_index()
			);

			class_file::constant::name_and_type_index nat_index {
				(uint16) consts.size()
			};
			consts.emplace_back(class_file::constant::name_and_type {
				.name_index = name_index,
				.descriptor_index = descriptor_index
			});

			class_file::constant::field_ref_index field_ref_index {
				(uint16) consts.size()
			};
			consts.emplace_back(class_file::constant::field_ref {
				.class_index{ 0 },
				.name_and_type_index = nat_index
			});

			uint8 load_index = (uint8) (param_index + 1);

			if constexpr(same_as_any<Type,
				class_file::object, class_file::array
			>) {
				instruction::write(
					os,
					instruction::a_load{ .index = load_index },
					begin
				);
			}
			else if constexpr(same_as<Type, class_file::d>) {
				instruction::write(
					os,
					instruction::d_load{ .index = load_index },
					begin
				);
			}
			else if constexpr(same_as<Type, class_file::j>) {
				instruction::write(
					os,
					instruction::l_load{ .index = load_index },
					begin
				);
			}
			else if constexpr(same_as<Type, class_file::f>) {
				instruction::write(
					os,
					instruction::f_load{ .index = load_index },
					begin
				);
			}
			else {
				instruction::write(
					os,
					instruction::i_load{ .index = load_index },
					begin
				);
			}

			instruction::write(
				os,
				instruction::put_field{ .index = field_ref_index },
				begin
			);

		});});
	}

	method constructor {
		class_file::access_flags {
			class_file::access_flag::_public
		},
		c_string{ u8"<init>" },
		ctor_desc_const,
		code_or_native_function_ptr{native_function_ptr{nullptr}},
		posix::memory<class_file::attribute::code::exception_handler>{},
		posix::memory<tuple<uint16, class_file::line_number>>{}
	};

}