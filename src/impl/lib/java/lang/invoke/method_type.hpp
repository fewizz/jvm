#include "decl/lib/java/lang/invoke/method_type.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/array.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/class.hpp"

#include <c_string.hpp>
#include <range.hpp>
#include <ranges.hpp>
#include <span.hpp>

template<
	range_of<c&> ParamClasses,
	range_of_decayed<utf8::unit> Descriptor
>
[[nodiscard]] inline expected<reference, reference> try_create_method_type(
	c& ret_class,
	ParamClasses&& params_classes,
	Descriptor&& descriptor
) {
	expected<reference, reference> possible_params_array_ref
		= try_create_array_of(
			class_class->get_array_class(),
			range_size(params_classes)
		);
	if(possible_params_array_ref.is_unexpected()) {
		return unexpected{ possible_params_array_ref.move_unexpected() };
	}
	reference params_array_ref = possible_params_array_ref.move_expected();

	params_classes
		.transform_view([](c& c) -> object& { return c.object(); })
		.copy_to(array_as_span<reference>(params_array_ref));

	expected<reference, reference> possible_descriptor_ref
		= try_create_byte_array(range_size(descriptor));
	
	if(possible_descriptor_ref.is_unexpected()) {
		return unexpected{ possible_descriptor_ref.move_unexpected() };
	}

	reference descriptor_ref = possible_descriptor_ref.move_expected();

	descriptor.copy_to(array_as_span<uint8>(descriptor_ref));

	expected<reference, reference> possible_mt
		= try_create_object(
			j::method_type::constructor.get(),
			reference{ ret_class.object() },
			move(params_array_ref),
			move(descriptor_ref)
		);
	
	if(possible_mt.is_unexpected()) {
		return unexpected{ possible_mt.move_unexpected() };
	}

	reference mt = possible_mt.move_expected();

	return mt;
}

template<
	range_of<c&> ParamsClasses,
	typename Handler
>
static decltype(auto) method_type_view_descriptor_utf8(
	c& ret, ParamsClasses&& params_classes, Handler&& handler
) {
	auto params = range { params_classes }
		.transform_view(
			[&](c& c) -> span<const utf8::unit> {
				return c.descriptor();
			}
		)
		.flat_view();

	auto descriptor = ranges {
			array{ u8'(' },
			params,
			array{ u8')' },
			ret.descriptor()
		}.concat_view();

	return descriptor
		.view_copied_elements_on_stack([&](span<const utf8::unit> descriptor)
			-> decltype(auto)
		{
			return handler(descriptor);
		}
	);
}

template<range_of<c&> ParamClasses>
[[nodiscard]] expected<reference, reference> try_create_method_type(
	c& ret_class,
	ParamClasses&& params_classes
) {
	return method_type_view_descriptor_utf8(
		ret_class,
		params_classes,
		[&](auto desc) {
			return try_create_method_type(
				ret_class,
				forward<ParamClasses>(params_classes),
				desc
			);
		}
	);
}

static void init_java_lang_invoke_method_type() {
	j::method_type::c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/invoke/MethodType"s
	);

	j::method_type::constructor = j::method_type::c->instance_methods().find(
		u8"<init>"s,
		u8"(Ljava/lang/Class;[Ljava/lang/Class;[B)V"s
	);

	j::method_type::parameter_types_instance_field_position =
		j::method_type::c->instance_field_position(
			u8"parameterTypes_"s,
			u8"[Ljava/lang/Class;"s
		);

	j::method_type::return_type_instance_field_position =
		j::method_type::c->instance_field_position(
			u8"returnType_"s,
			u8"Ljava/lang/Class;"s
		);

	j::method_type::descriptor_instance_field_position =
		j::method_type::c->instance_field_position(
			u8"descriptorUTF8_"s,
			u8"[B"s
		);

	j::method_type::c->declared_static_methods().find(
		u8"descriptorUTF8"s,
		u8"(Ljava/lang/Class;[Ljava/lang/Class;)[B"s
	).native_function(
		(void*)+[](
			native_environment*,
			j::c* ret_class,
			object* params_class_array
		) -> object*
		{
			auto params
				= array_as_span<reference>(*params_class_array)
				.transform_view([](reference& class_ref) -> c& {
					return class_from_class_instance(class_ref);
				});

			expected<reference, reference> possible_result
			= method_type_view_descriptor_utf8(
				ret_class->get_c(),
				params,
				[](auto destriptor) -> expected<reference, reference> {
					expected<reference, reference> possible_array
						= try_create_byte_array(destriptor.size());
					if(possible_array.is_unexpected()) {
						return unexpected {
							possible_array.move_unexpected()
						};
					}
					reference array = possible_array.move_expected();
					destriptor.copy_to(array_as_span<utf8::unit>(array));
					return array;
				}
			);
			if(possible_result.is_unexpected()) {
				thrown_in_native = possible_result.move_unexpected();
				return nullptr;
			}
			reference result = possible_result.move_expected();
			return & result.unsafe_release_without_destroing();
		}
	);
}