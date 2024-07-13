#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/call_site.hpp"
#include "decl/lib/java/lang/invoke/constant_call_site.hpp"
#include "decl/lib/jvm/mh/constructor.hpp"
#include "decl/lib/java/lang/string.hpp"

#include <optional.hpp>
#include <class_file/attribute/code/write_instruction.hpp>

static nuint lambda_index = 0;

struct constants_and_data_builder {
	class_data data{};
	list<blocky_memory<::constant, 64>> constants{};

	template<typename Type, typename TypeIndex = Type::index_type>
	tuple<Type, TypeIndex> add(Type constant) {
		typename Type::index_type index {
			(uint16) (constants.emplace_back_and_get_index(constant) + 1)
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
		span s = data.back().casted<utf8::unit>();
		range{ r }.copy_to(s);
		return add<Type, TypeIndex>(s);
	};

	nuint size() const { return constants.size(); }

	::constants bake_constants() {
		posix::memory<constant, uint16> result
			= posix::allocate<constant, uint16>(constants.size());
		for (auto [index, cnst] : constants.indexed_view()) {
			new (&result[index]) constant(cnst);
		}
		return move(result);
	}

};

template<basic_range Name>
expected<c&, reference> try_define_lamda_class(
	Name&& class_name,
	c& interface,
	j::string& interface_method_name,
	j::method_type& constructor_mt,
	j::method_type& interface_mt,
	j::method_handle& implementation,
	[[maybe_unused]] j::method_type& dynamic_mt,
	j::c_loader* defining_loader
) {
	constants_and_data_builder consts{};

	auto [this_class_name, this_class_name_index] = consts.add_with_data<
		class_file::constant::name, class_file::constant::name_index
	>(class_name);

	auto [this_class_descriptor, this_class_descriptor_index]
		= consts.add_with_data<class_file::constant::descriptor>(
			ranges {
				u8"L"sv,
				this_class_name,
				u8";"sv
			}.concat_view()
		);

	auto [super_class_name, super_class_name_index]
		= consts.add(class_file::constant::name {
			u8"java/lang/Object"sv
		});

	auto [this_class, this_class_index] = 
		consts.add(class_file::constant::_class {
			.name_constant_index = this_class_name_index
		});

	auto [super_class, super_class_index] =
		consts.add(class_file::constant::_class {
			.name_constant_index = super_class_name_index
		});

	auto [this_class_ctor_name, this_class_ctor_name_index] =
		consts.add(class_file::constant::name {
			u8"<init>"sv
		});

	auto [super_class_ctor_desc, super_class_ctor_desc_index] =
		consts.add(class_file::constant::descriptor {
			u8"()V"sv
		});

	auto [super_class_ctor_nat, super_class_ctor_nat_index] =
		consts.add(
			class_file::constant::name_and_type {
				.name_constant_index = this_class_ctor_name_index,
				.descriptor_constant_index = super_class_ctor_desc_index
			}
		);

	auto [super_class_ctor_ref, super_class_ctor_ref_index] =
		consts.add(
			class_file::constant::method_ref {
				.class_constant_index = super_class_index,
				.name_and_type_constant_index = super_class_ctor_nat_index
			}
		);

	auto [this_class_ctor_descriptor, this_class_ctor_descriptor_index] =
		consts.add_with_data<class_file::constant::descriptor>(
			constructor_mt.descriptor()
		);

	::list declared_instance_fields {
		posix::allocate<instance_field, declared_instance_field_index>(
			constructor_mt.parameter_types_count()
		)
	};

	const auto fields_constants_beginning = consts.size() + 1;

	auto param_index_to_field_ref_index = [&](nuint index) {
		return class_file::constant::field_ref_index {
			(uint16) (fields_constants_beginning + index * 4 + 3)
		};
	};

	for (auto [param_index, param_c] : constructor_mt.parameter_types_view().indexed_view()) {
		// name
		auto [name, name_index] =
			consts.add_with_data<class_file::constant::name>(
				ranges {
					u8"captured_"sv,
					array{ utf8::unit('0' + param_index) }
				}.concat_view()
			); // +0

		// descriptor
		auto [descriptor, descriptor_index] =
			consts.add_with_data<class_file::constant::descriptor>(
				param_c.descriptor()
			); // +1

		// name and type
		auto [nat, nat_index] =
			consts.add(
				class_file::constant::name_and_type {
					.name_constant_index = name_index,
					.descriptor_constant_index = descriptor_index
				}
			); // +2

		consts.add(
			class_file::constant::field_ref {
				.class_constant_index = this_class_index,
				.name_and_type_constant_index = nat_index
			}
		); // +3

		declared_instance_fields.emplace_back(
			class_file::access_flags{ class_file::access_flag::_private },
			name, descriptor
		);
	}

	// name
	auto [mh_name, mh_name_index] = consts.add(
		class_file::constant::name{ u8"method_handle"sv }
	); // +0

	// descriptor
	auto [mh_descriptor, mh_descriptor_index] = consts.add(
		class_file::constant::descriptor {
			u8"Ljava/lang/invoke/MethodHandle;"sv
		}
	); // +1

	// nat
	auto [mh_nat, mh_nat_index] = consts.add(
		class_file::constant::name_and_type {
			.name_constant_index = mh_name_index,
			.descriptor_constant_index = mh_descriptor_index
		}
	); // +2

	auto [mh_field_ref, mh_field_ref_index] = consts.add(
		class_file::constant::field_ref {
			.class_constant_index = this_class_index,
			.name_and_type_constant_index = mh_nat_index
		}
	); // +3

	auto declared_static_fields
		= posix::allocate<static_field, declared_static_field_index>(1);
	new (&declared_static_fields[declared_static_field_index{0}]) static_field {
		class_file::access_flags {
			class_file::access_flag::_private,
			class_file::access_flag::_static
		},
		mh_name, mh_descriptor
	};

	::list constructor_code = posix::allocate<>(1024);
	{
		auto os = constructor_code.output_stream();

		using namespace class_file::attribute::code;

		// load *this*
		instruction::write(
			instruction::a_load_0{}, os
		);
		// invoke super constructor
		instruction::write(
			instruction::invoke_special { super_class_ctor_ref_index }, os
		);

		// set every field
		for (
			auto [param_index, param_c] :
			constructor_mt.parameter_types_view().indexed_view()
		) {
			param_c.view_non_void_raw_type([&]<typename Type>
		{
			// load *this*, for *put_field*
			instruction::write(
				instruction::a_load_0{}, os
			);

			// first is *this*
			uint8 load_index = (uint8) (param_index + 1);

			if constexpr(same_as<Type, reference>) {
				instruction::write(
					instruction::a_load{ .index = load_index }, os
				);
			}
			else if constexpr(same_as<Type, double>) {
				instruction::write(
					instruction::d_load{ .index = load_index }, os
				);
			}
			else if constexpr(same_as<Type, int64>) {
				instruction::write(
					instruction::l_load{ .index = load_index }, os
				);
			}
			else if constexpr(same_as<Type, float>) {
				instruction::write(
					instruction::f_load{ .index = load_index }, os
				);
			}
			else if constexpr(
				same_as_any<Type, int32, int16, uint16, int8, bool>
			) {
				instruction::write(
					instruction::i_load{ .index = load_index }, os
				);
			}
			else {
				[]<bool b = false>{static_assert(b);};
			}

			instruction::write(
				instruction::put_field {
					.field_ref_constant_index
						= param_index_to_field_ref_index(param_index)
				},
				os
			);

		});}

		instruction::write(
			instruction::_return{}, os
		);
	}

	::list impl_code = posix::allocate<>(1024);
	{
		auto os = impl_code.output_stream();

		using namespace class_file::attribute::code;

		// load method handle
		instruction::write(
			instruction::get_static {
				.field_ref_constant_index = mh_field_ref_index
			},
			os
		);

		// load captured variables
		for (
			nuint param_index :
			constructor_mt.parameter_types_view().index_view()
		) {
			// load *this*
			instruction::write(
				instruction::a_load_0{}, os
			);

			instruction::write(
				instruction::get_field {
					.field_ref_constant_index
						= param_index_to_field_ref_index(param_index)
				},
				os
			);
		}

		// load args
		for (
			auto [param_index, param_c] :
			interface_mt.parameter_types_view().indexed_view()
		) {
			param_c.view_non_void_raw_type([&]<typename Type>
		{

			// first is *this*
			uint8 load_index = (uint8) (param_index + 1);

			if constexpr(same_as<Type, reference>) {
				instruction::write(
					instruction::a_load{ .index = load_index },
					os
				);
			}
			else if constexpr(same_as<Type, double>) {
				instruction::write(
					instruction::d_load{ .index = load_index },
					os
				);
			}
			else if constexpr(same_as<Type, int64>) {
				instruction::write(
					instruction::l_load{ .index = load_index },
					os
				);
			}
			else if constexpr(same_as<Type, float>) {
				instruction::write(
					instruction::f_load{ .index = load_index },
					os
				);
			}
			else if constexpr(
				same_as_any<Type, int32, int16, uint16, int8, bool>
			) {
				instruction::write(
					instruction::i_load{ .index = load_index },
					os
				);
			}
			else {
				[]<bool b = false>{ static_assert(b); }();
			}

		});}

		auto [mh_class_name, mh_class_name_index] =
			consts.add_with_data<class_file::constant::name>(
				u8"java/lang/invoke/MethodHandle"sv
			);

		auto [mh_class, mh_class_index] = consts.add(
			class_file::constant::_class {
				.name_constant_index = mh_class_name_index,
			}
		);

		auto [mh_invoke_method_name, mh_invoke_method_name_index] =
			consts.add_with_data<class_file::constant::name>(
				u8"invoke"sv
			);
		auto [mh_invoke_method_desc, mh_invoke_method_desc_index] =
			consts.add_with_data<class_file::constant::descriptor>(
				implementation.method_type().descriptor()
			);

		auto [mh_invoke_method_nat, mh_invoke_method_nat_index] = consts.add(
			class_file::constant::name_and_type {
				.name_constant_index = mh_invoke_method_name_index,
				.descriptor_constant_index = mh_invoke_method_desc_index
			}
		);

		auto [mh_invoke_method_ref, mh_invoke_method_ref_index] = consts.add(
			class_file::constant::method_ref {
				.class_constant_index = mh_class_index,
				.name_and_type_constant_index = mh_invoke_method_nat_index
			}
		);

		// invoke method handle
		instruction::write(
			instruction::invoke_virtual {
				.method_ref_constant_index = mh_invoke_method_ref_index
			},
			os
		);

		for(c& param_c : interface_mt.parameter_types_view())
		{ param_c.view_raw_type([&]<typename Type> {
			if constexpr(same_as<Type, reference>) {
				instruction::write(instruction::a_return{}, os);
			}
			else if constexpr(same_as<Type, double>) {
				instruction::write(instruction::d_return{}, os);
			}
			else if constexpr(same_as<Type, int64>) {
				instruction::write(instruction::l_return{}, os);
			}
			else if constexpr(same_as<Type, float>) {
				instruction::write(instruction::f_return{}, os);
			}
			else if constexpr(
				same_as_any<Type, int32, int16, uint16, int8, bool>
			) {
				instruction::write(instruction::i_return{}, os);
			}
			else if constexpr(
				same_as<Type, void_t>
			) {
				instruction::write(instruction::_return{}, os);
			}
			else {
				[]<bool b = false>{ static_assert(b); }();
			}
		});}

	}

	auto declared_instance_methods
		= posix::allocate<instance_method, declared_instance_method_index>(2);

	new (&declared_instance_methods[declared_instance_method_index{0}]) ::instance_method {
		class_file::access_flags {
			class_file::access_flag::_public
		},
		this_class_ctor_name,
		this_class_ctor_descriptor,
		code_or_native_function_ptr { code {
			consts.data.emplace_back(
				constructor_code.move_storage_range()
			).as_span(),
			128, // max stack
			128  // max locals
		}},
		posix::memory<class_file::attribute::code::exception_handler>{},
		posix::memory<tuple<uint16, class_file::line_number>>{}
	};

	auto [impl_method_name, impl_method_name_index]
		= interface_method_name.view_on_stack_as_utf8(
			[&](span<utf8::unit> str) {
				return consts.add_with_data<class_file::constant::name>(
					str
				);
			}
		);

	auto [impl_method_desc, impl_method_desc_index]
		= consts.add_with_data<class_file::constant::descriptor>(
			interface_mt.descriptor()
		);

	new (&declared_instance_methods[declared_instance_method_index{1}]) ::instance_method {
		class_file::access_flags {
			class_file::access_flag::_public
		},
		impl_method_name,
		impl_method_desc,
		code_or_native_function_ptr { code {
			consts.data.emplace_back(
				impl_code.move_storage_range()
			).as_span(),
			128, // max stack
			128  // max locals
		}},
		posix::memory<class_file::attribute::code::exception_handler>{},
		posix::memory<tuple<uint16, class_file::line_number>>{}
	};

	auto interfaces = posix::allocate<c*>(1);
	interfaces[0] = &interface;

	c& c = classes.emplace_back(
		move(consts.bake_constants()),
		bootstrap_methods{},
		move(consts.data),
		class_file::access_flags {
			class_file::access_flag::_public
		},
		this_class_name,
		this_class_descriptor,
		class_file::constant::utf8{}, // source file name
		object_class, // super
		move(interfaces),
		move(declared_static_fields),
		move(declared_instance_fields.move_storage_range()),
		posix::memory<static_method, declared_static_method_index>{}, // declared static methods
		move(declared_instance_methods),
		optional<method>{}, // initialisation method
		is_array_class{ false },
		is_primitive_class{ false },
		defining_loader == nullptr ? nullptr_ref : reference{ *defining_loader }
	);

	optional<reference> possible_throwable = c.try_initialise_if_need();
	if(possible_throwable.has_value()) {
		return possible_throwable.move();
	}

	layout::position mh_position
		= c.declared_static_field_position(declared_static_field_index{0});
	
	c.set(mh_position, reference{ implementation });
	return c;
}

static void init_java_lang_invoke_lambda_meta_factory() {

	c& c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/invoke/LambdaMetafactory"sv
	);

	c.declared_static_methods().find(
		u8"metafactory"sv,
		u8"("
			"Ljava/lang/invoke/MethodHandles$Lookup;"
			"Ljava/lang/String;"
			"Ljava/lang/invoke/MethodType;"
			"Ljava/lang/invoke/MethodType;"
			"Ljava/lang/invoke/MethodHandle;"
			"Ljava/lang/invoke/MethodType;"
		")"
		"Ljava/lang/invoke/CallSite;"sv
	).native_function(
		(void*)+[](
			native_environment*,
			[[maybe_unused]] object* caller,
			j::string* interface_method_name,
			j::method_type* factory_type,
			j::method_type* interface_method_type,
			j::method_handle* implementation,
			j::method_type* dynamic_method_type
		) -> j::call_site* {
			expected<reference, reference> possible_constructor_mt
				= try_create_method_type(
					void_class.get(), // return type,
					factory_type->parameter_types_view()
				);
			if(possible_constructor_mt.is_unexpected()) {
				thrown_in_native = possible_constructor_mt.move_unexpected();
				return nullptr;
			}

			reference constructor_mt_ref
				= possible_constructor_mt.move_expected();

			j::method_type& constructor_mt
				= (j::method_type&) constructor_mt_ref.object();

			auto name = ranges {
				u8"lambda_"sv,
				array{ utf8::unit('0' + lambda_index) }
			}.concat_view();

			expected<::c&, reference> possible_c
				= classes.lock_or_throw_linkage_error(
					name,
					nullptr, // defining loader
					[&] {
						return try_define_lamda_class(
							name,
							factory_type->return_type(),
							*interface_method_name,
							constructor_mt,
							*interface_method_type,
							*implementation,
							*dynamic_method_type,
							nullptr
						);
					}
				);
			if(possible_c.is_unexpected()) {
				thrown_in_native = possible_c.move_unexpected();
				return nullptr;
			}

			::c& c = possible_c.get_expected();
			instance_method& constructor = c.instance_methods().find(
				u8"<init>"sv, constructor_mt.descriptor()
			);

			expected<reference, reference> possible_mh
				= try_create_constructor_mh(
					*factory_type, constructor
				);
			if(possible_mh.is_unexpected()) {
				thrown_in_native = possible_mh.move_unexpected();
				return nullptr;
			}
			reference mh = possible_mh.move_expected();

			expected<reference, reference> possible_cs
				= j::constant_call_site::try_create(
					(j::method_handle&)mh.object()
				);
			if(possible_cs.is_unexpected()) {
				thrown_in_native = possible_cs.move_unexpected();
				return nullptr;
			}

			return (j::call_site*)
				& possible_cs.move_expected()
				.unsafe_release_without_destroing();
		}
	);

}