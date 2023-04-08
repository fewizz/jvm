#include "decl/lib/java/lang/invoke/method_type.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/array.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/execute.hpp"

#include <c_string.hpp>
#include <range.hpp>
#include <ranges.hpp>
#include <span.hpp>

static optional<method&> method_type_constructor;

template<
	range_of<_class&> ParamClasses,
	range_of_decayed<char> Descriptor
>
[[nodiscard]]inline expected<reference, reference> try_create_method_type(
	_class& ret_class,
	ParamClasses&& params_classes,
	Descriptor&& descriptor
) {
	expected<reference, reference> possible_params_array_ref
		= try_create_array_of(
			class_class->get_array_class(),
			range_size(params_classes)
		);
	if(possible_params_array_ref.is_unexpected()) {
		return unexpected{ move(possible_params_array_ref.get_unexpected()) };
	}
	reference params_array_ref = move(possible_params_array_ref.get_expected());

	params_classes
		.transform_view([](_class& c) { return c.instance(); })
		.copy_to(array_as_span<reference>(params_array_ref));

	expected<reference, reference> possible_descriptor_ref
		= try_create_byte_array(range_size(descriptor));
	
	if(possible_descriptor_ref.is_unexpected()) {
		return unexpected{ move(possible_descriptor_ref.get_unexpected()) };
	}

	reference descriptor_ref = move(possible_descriptor_ref.get_expected());

	descriptor.copy_to(array_as_span<uint8>(descriptor_ref));

	expected<reference, reference> possible_mt
		= try_create_object(
			method_type_constructor.get(),
			ret_class.instance(),
			move(params_array_ref),
			move(descriptor_ref)
		);
	
	if(possible_mt.is_unexpected()) {
		return unexpected{ move(possible_mt.get_unexpected()) };
	}

	reference mt = move(possible_mt.get_expected());

	return mt;
}

template<
	range_of<_class&> ParamsClasses,
	typename Handler
>
static decltype(auto) method_type_view_descriptor_utf8(
	_class& ret, ParamsClasses&& params_classes, Handler&& handler
) {
	return range { params_classes }
		.transform_view(
			[&](_class& c) -> span<const char> {
				return c.descriptor();
			}
		)
		.flat_view()
		.sized_view()
		.view_copied_elements_on_stack([&](span<const char> params)
			-> decltype(auto)
		{
			auto descriptor = ranges {
				array{ '(' },
				params,
				array{ ')' },
				ret.descriptor()
			}.concat_view().sized_view();

			return handler(descriptor);
		}
	);
}

template<range_of<_class&> ParamClasses>
[[nodiscard]] expected<reference, reference> try_create_method_type(
	_class& ret_class,
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
	method_type_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/invoke/MethodType" }
	);

	method_type_constructor = method_type_class->instance_methods().find(
		c_string{"<init>"},
		c_string{"(Ljava/lang/Class;[Ljava/lang/Class;[B)V"}
	);

	method_type_parameter_types_instance_field_position =
		method_type_class->instance_field_position(
			c_string{ "parameterTypes_" },
			c_string{ "[Ljava/lang/Class;" }
		);

	method_type_return_type_instance_field_position =
		method_type_class->instance_field_position(
			c_string{ "returnType_" },
			c_string{ "Ljava/lang/Class;" }
		);

	method_type_descriptor_instance_field_position =
		method_type_class->instance_field_position(
			c_string{ "descriptorUTF8_" },
			c_string{ "[B" }
		);

	method_type_class->declared_methods().find(
		c_string{ "descriptorUTF8" },
		c_string{ "(Ljava/lang/Class;[Ljava/lang/Class;)[B" }
	).native_function(
		(void*)+[](
			native_environment*, object* ret_class, object* params_class_array
		) -> object* {

			auto params
				= array_as_span<reference>(*params_class_array)
				.transform_view([](reference& class_ref) -> _class& {
					return class_from_class_instance(class_ref);
				});

			expected<reference, reference> possible_result
			= method_type_view_descriptor_utf8(
				class_from_class_instance(*ret_class),
				params,
				[](auto destriptor) -> expected<reference, reference> {
					expected<reference, reference> possible_array
						= try_create_byte_array(destriptor.size());
					if(possible_array.is_unexpected()) {
						return unexpected {
							move(possible_array.get_unexpected())
						};
					}
					reference array = move(possible_array.get_expected());
					destriptor.copy_to(array_as_span<char>(array));
					return array;
				}
			);
			if(possible_result.is_unexpected()) {
				thrown_in_native = move(possible_result.get_unexpected());
				return nullptr;
			}
			reference result = move(possible_result.get_expected());
			return & result.unsafe_release_without_destroing();
		}
	);
}

static span<const char> method_type_descriptor(object& mt) {
	reference& utf8_desc = mt.get<reference>(
		method_type_descriptor_instance_field_position
	);
	return array_as_span<const char>(utf8_desc);
}