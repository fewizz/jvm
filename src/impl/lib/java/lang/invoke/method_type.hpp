#include "decl/lib/java/lang/invoke/method_type.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/array.hpp"
#include "decl/native/environment.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/execute.hpp"

#include <c_string.hpp>
#include <range.hpp>
#include <span.hpp>

static optional<method&> method_type_constructor;

template<
	range_of<_class&> ParamClasses,
	range_of_decayed<char> Descriptor
>
static reference create_method_type(
	_class& ret_class,
	ParamClasses&& params_classes,
	Descriptor&& descriptor
) {
	reference params_array_ref = create_array_of(
		class_class->get_array_class(),
		range_size(params_classes)
	);

	params_classes
		.transform_view([](_class& c) { return c.instance(); })
		.copy_to(array_as_span<reference>(params_array_ref));

	reference descriptor_ref = create_byte_array(range_size(descriptor));
	descriptor.copy_to(array_as_span<uint8>(descriptor_ref));

	reference result = create_object(method_type_class.get());

	execute(
		method_type_constructor.get(),
		result,
		ret_class.instance(),
		params_array_ref,
		descriptor_ref
	);

	return result;
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
static reference create_method_type(
	_class& ret_class,
	ParamClasses&& params_classes
) {
	return method_type_view_descriptor_utf8(
		ret_class,
		params_classes,
		[&](auto desc) {
			return create_method_type(
				ret_class,
				forward<ParamClasses>(params_classes),
				desc
			);
		}
	);
}

static void init_java_lang_invoke_method_type() {
	method_type_class = classes.find_or_load(
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

			return & method_type_view_descriptor_utf8(
				class_from_class_instance(*ret_class),
				params,
				[](auto destriptor) {
					reference array = create_byte_array(destriptor.size());
					destriptor.copy_to(array_as_span<char>(array));
					return array;
				}
			).unsafe_release_without_destroing();
		}
	);
}

static span<const char> method_type_descriptor(object& mt) {
	reference& utf8_desc = mt.get<reference>(
		method_type_descriptor_instance_field_position
	);
	return array_as_span<const char>(utf8_desc);
}