#pragma once

#include "decl/classes.hpp"
#include "decl/lib/java/lang/object.hpp"

#include <range.hpp>
#include <ranges.hpp>

#include <posix/memory.hpp>

#include <class_file/attribute/code/writer.hpp>

struct constants_builder {
	class_data& data;
	list<blocky_memory<::constant, 64>> constants{};

	template<typename Type, typename TypeIndex = Type::index_type>
	tuple<Type, TypeIndex> add(Type constant) {
		typename Type::index_type index {
			(uint16) constants.emplace_back_and_get_index(constant)
		};
		return { constant, index };
	};

	template<
		typename Type,
		typename TypeIndex = Type::index_type,
		range_of_decayed<utf8::unit> Range
	>
	auto add_with_data(Range&& r) {
		data.emplace_back(posix::allocate(range_size(r)));
		span s = data.back().cast<utf8::unit>();
		range{ r }.copy_to(s);
		return add<Type, TypeIndex>(s);
	};

	nuint size() const { return constants.size(); }

};

template<
	basic_range Name,
	basic_range CtorDescriptor,
	basic_range IDescriptor
>
expected<c&, reference> classes::try_define_lamda_class(
	Name&& class_name_param,
	c& interface_to_implement,
	CtorDescriptor&& ctor_descriptor_param,
	IDescriptor&& method_descriptor_param,
	j::c_loader* defining_loader // L
) {
	mutex_->lock();
	on_scope_exit unlock_mutex { [&] { mutex_->unlock(); }};

	/* 1. First, the Java Virtual Machine determines whether L has already
	      been recorded as an initiating loader of a class or interface
	      denoted by N. If so, this derivation attempt is invalid and
	      derivation throws a LinkageError. */
	{
		optional<c&> c
			= try_find_class_which_loading_was_initiated_by(
				class_name_param,
				defining_loader
			);
		if(c.has_value()) {
			return unexpected { try_create_linkage_error().get() };
		}
	}

	auto read_parameter_types = [](
		auto descriptor
	) -> list<posix::memory<one_of_descriptor_parameter_types>> {
		class_file::method_descriptor::reader reader {
			descriptor.iterator()
		};
		uint8 count = reader.try_read_parameters_count(
			[]([[maybe_unused]] auto err) { posix::abort(); }
		).get();

		::list list = posix::allocate<one_of_descriptor_parameter_types>(count);

		reader.try_read_parameter_types_and_get_return_type_reader(
			[&]<typename ParamType>(ParamType parameter_type) {
				list.emplace_back(parameter_type);
			},
			[](auto) { posix::abort(); }
		);

		return move(list);
	};

	list<posix::memory<one_of_descriptor_parameter_types>>
		ctor_parameters = read_parameter_types(ctor_descriptor_param);
	list<posix::memory<one_of_descriptor_parameter_types>>
		method_parameters = read_parameter_types(method_descriptor_param);
	
	class_data data{};
	constants_builder consts{ .data = data };

	auto [this_class_name, this_class_name_index] = consts.add_with_data<
		class_file::constant::utf8, class_file::constant::name_index
	>(class_name_param);

	auto [this_class_descriptor, this_class_descriptor_index]
		= consts.add_with_data<class_file::constant::utf8>(
			ranges{ c_string{"L"}, this_class_name, c_string{";"} }
		);

	auto [super_class_name, super_class_name_index]
		= consts.add(class_file::constant::name {
			c_string{ u8"java/lang/Object" }
		});

	auto [this_class, this_class_index] = 
		consts.add(class_file::constant::_class {
			.name_index = this_class_name_index
		});

	auto [super_class, super_class_index] =
		consts.add(class_file::constant::_class {
			.name_index = super_class_name_index
		});

	auto [this_class_ctor_name, this_class_ctor_name_index] =
		consts.add(class_file::constant::name {
			c_string{ u8"<init>" }
		});

	auto [super_class_ctor_desc, super_class_ctor_desc_index] =
		consts.add(class_file::constant::descriptor {
			c_string{ u8"()V" }
		});

	auto [super_class_ctor_nat, super_class_ctor_nat_index] =
		consts.add(
			class_file::constant::name_and_type {
				.name_index = this_class_ctor_name_index,
				.descriptor_index = super_class_ctor_desc_index
			}
		);

	auto [super_class_ctor_ref, super_class_ctor_ref_index] =
		consts.add(
			class_file::constant::method_ref {
				.class_index = super_class_index,
				.name_and_type_index = super_class_ctor_nat_index
			}
		);

	auto [this_class_ctor_descriptor, this_class_ctor_descriptor_index] =
		consts.add_with_data<class_file::constant::descriptor>(
			ctor_descriptor_param
		);

	::list declared_instance_fields {
		posix::allocate<instance_field>(ctor_parameters.size())
	};

	const auto fields_constants_beginning = consts.size();

	auto param_index_to_field_ref_index = [&](nuint index) {
		return fields_constants_beginning + index * 4 + 3;
	};

	ctor_parameters.for_each_indexed([&](
		auto param, nuint param_index
	) { param.view_type([&]<typename Type>() {
		// name
		auto [name, name_index] =
			consts.add_with_data<class_file::constant::name>(
				ranges {
					c_string{ u8"captured_" },
					array{ utf8::unit('0' + param_index) }
				}.concat_view()
			);

		// descriptor
		auto [descriptor, descriptor_index] =
			consts.add_with_data<class_file::constant::descriptor>(
				param.utf8_string()
			); // +1

		// name and type
		auto [nat, nat_index] =
			consts.add(
				class_file::constant::name_and_type {
					.name_index = name_index,
					.descriptor_index = descriptor_index
				}
			); // +2

		consts.add(
			class_file::constant::field_ref {
				.class_index{ this_class_index },
				.name_and_type_index = nat_index
			}
		); // +3

		declared_instance_fields.emplace_back(
			class_file::access_flags{ class_file::access_flag::_private },
			name, descriptor
		);
	});});

	auto declared_static_fields = posix::allocate<static_field>(1);
	// name
	auto [mh_name, mh_name_index] = consts.add(
		class_file::constant::name{ c_string{ u8"method_handle" } }
	); // +0

	// descriptor
	auto [mh_descriptor, mh_descriptor_index] = consts.add(
		class_file::constant::descriptor {
			c_string{ u8"Ljava/lang/invoke/MethodHandle;" }
		}
	); // +1

	// nat
	auto [mh_nat, mh_nat_index] = consts.add(
		class_file::constant::name_and_type {
			.name_index = mh_name_index,
			.descriptor_index = mh_descriptor_index
		}
	); // +2

	auto [mh_field_ref, mh_field_ref_index] = consts.add(
		class_file::constant::field_ref {
			.class_index = this_class_index,
			.name_and_type_index = mh_nat_index
		}
	); // +3

	new (&declared_static_fields[0]) static_field {
		class_file::access_flags{ class_file::access_flag::_private },
		this_class_name, this_class_descriptor
	};

	::list constructor_code = posix::allocate<>(1024);
	{
		auto os = constructor_code.output_stream();

		using namespace class_file::attribute::code;

		// load *this*
		instruction::write(
			os, instruction::a_load_0{}
		);
		// invoke super constructor
		instruction::write(
			os, instruction::invoke_special { super_class_ctor_ref_index }
		);

		// set every field
		ctor_parameters.for_each_indexed([&](
			auto param, nuint param_index
		) { param.view_type([&]<typename Type>() {

			// first is *this*
			uint8 load_index = (uint8) (param_index + 1);

			if constexpr(same_as<Type, class_file::object>) {
				instruction::write(
					os, instruction::a_load{ .index = load_index }
				);
			}
			else if constexpr(same_as<Type, class_file::d>) {
				instruction::write(
					os, instruction::d_load{ .index = load_index }
				);
			}
			else if constexpr(same_as<Type, class_file::j>) {
				instruction::write(
					os, instruction::l_load{ .index = load_index }
				);
			}
			else if constexpr(same_as<Type, class_file::f>) {
				instruction::write(
					os, instruction::f_load{ .index = load_index }
				);
			}
			else {
				instruction::write(
					os, instruction::i_load{ .index = load_index }
				);
			}

			instruction::write(
				os,
				instruction::put_field {
					.index = param_index_to_field_ref_index(param_index)
				}
			);

		});});
	}

	::list impl_code = posix::allocate<>(1024);
	{
		auto os = constructor_code.output_stream();

		using namespace class_file::attribute::code;

		// load method handle
		instruction::write(
			os,
			instruction::get_static {
				.index = mh_field_ref_index
			}
		);

		ctor_parameters.for_each_indexed([&](
			auto param, nuint param_index
		) { param.view_type([&]<typename Type>() {
			instruction::write(
				os,
				instruction::get_field {
					.index = param_index_to_field_ref_index(param_index)
				}
			);

		});});

		// set every field
		method_parameters.for_each_indexed([&](
			auto param, nuint param_index
		) { param.view_type([&]<typename Type>() {

			// first is *this*
			uint8 load_index = (uint8) (param_index + 1);

			if constexpr(same_as<Type, class_file::object>) {
				instruction::write(
					os,
					instruction::a_load{ .index = load_index }
				);
			}
			else if constexpr(same_as<Type, class_file::d>) {
				instruction::write(
					os,
					instruction::d_load{ .index = load_index }
				);
			}
			else if constexpr(same_as<Type, class_file::j>) {
				instruction::write(
					os,
					instruction::l_load{ .index = load_index }
				);
			}
			else if constexpr(same_as<Type, class_file::f>) {
				instruction::write(
					os,
					instruction::f_load{ .index = load_index }
				);
			}
			else {
				instruction::write(
					os,
					instruction::i_load{ .index = load_index }
				);
			}

			instruction::write(
				os,
				instruction::put_field {
					.index = param_index_to_field_ref_index(param_index)
				}
			);

		});});

		auto [mh_class_name, mh_class_name_index] =
			consts.add_with_data<class_file::constant::name>(
				c_string{ u8"java/lang/invoke/MethodHandle" }
			);

		auto [mh_class, mh_class_index] = consts.add(
			class_file::constant::_class {
				.name_index = mh_class_name_index,
			}
		);

		auto [mh_method_name, mh_method_name_index] =
			consts.add_with_data<class_file::constant::name>(
				c_string{ u8"invoke" }
			);
		auto [mh_method_desc, mh_method_desc_index] =
			consts.add_with_data<class_file::constant::descriptor>(
				c_string{ u8"([Ljava/lang/Object;)Ljava/lang/Object;" }
			);

		auto [mh_method_nat, mh_method_nat_index] = consts.add(
			class_file::constant::name_and_type {
				.name_index = mh_method_name_index,
				.descriptor_index = mh_descriptor_index
			}
		);

		auto [mh_method_ref, mh_method_ref_index] = consts.add(
			class_file::constant::method_ref {
				.class_index = mh_class_index,
				.name_and_type_index = mh_method_nat_index
			}
		);

		instruction::write(
			os,
			instruction::invoke_virtual {
				.index = mh_method_ref_index
			}
		);

	}

	auto declared_instance_methods = posix::allocate<instance_method>(2);

	new (&declared_instance_methods[0]) ::instance_method {
		class_file::access_flags {
			class_file::access_flag::_public
		},
		this_class_descriptor,
		this_class_ctor_descriptor,
		code_or_native_function_ptr{native_function_ptr{nullptr}},
		posix::memory<class_file::attribute::code::exception_handler>{},
		posix::memory<tuple<uint16, class_file::line_number>>{}
	};

	auto interfaces = posix::allocate<c*>(1);
	interfaces[0] = &interface_to_implement;

	c& c = emplace_back(
		move(consts),
		bootstrap_methods{},
		move(data),
		class_file::access_flags {
			class_file::access_flag::_public
		},
		this_class_ctor_name,
		this_class_descriptor,
		class_file::constant::utf8{}, // source file name
		object_class.get(),
		move(interfaces),
		posix::memory<static_field>{}, // declared static fields
		move(declared_instance_fields),
		posix::memory<static_method>{}, // declared static methods
		move(declared_instance_methods),
		optional<method>{}, // initialisation method
		is_array_class{ false },
		is_primitive_class{ false },
		defining_loader == nullptr ? nullptr_ref : reference{ *defining_loader }
	);
}