#include "decl/lib/java/lang/invoke/method_type.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/array.hpp"
#include "decl/native/interface/environment.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/lib/java/lang/class.hpp"

#include <c_string.hpp>
#include <range.hpp>
#include <span.hpp>

template<range_of_decayed<_class*> ParamClasses, range_of_decayed<char> Descriptor>
static reference create_method_type(
	ParamClasses&& params_classes,
	_class& ret_class,
	Descriptor&& descriptor
) {
	reference params_array = create_array_of(
		class_class->get_array_class(),
		range_size(params_classes)
	);
	params_classes
		.transform_view([](_class* c) { return c->instance(); })
		.copy_to(array_as_span<reference>(params_array));

	reference descriptor0 = create_byte_array(range_size(descriptor));
	descriptor.copy_to(array_as_span<uint8>(descriptor0));

	reference o = create_object(method_type_class.value());
	o[method_type_parameter_types_instance_field_index] = params_array;
	o[method_type_return_type_instance_field_index] = ret_class.instance();
	o[method_type_descriptor_instance_field_index] = descriptor0;
	return o;
}

template<range_of<_class&> ParamsClasses>
static reference method_type_create_method_descriptor_utf8(
	ParamsClasses&& params_classes, _class& ret
) {
	return range{ params_classes }.transform_view(
		[&](_class& c) -> span<const char> {
			return c.descriptor();
		}
	)
		.flat_view()
		.sized_view()
		.view_copied_elements_on_stack(
	[&](span<const char> params) {
		auto descriptor = ranges {
			array{ '(' },
			params,
			array{ ')' },
			ret.descriptor()
		}.concat_view().sized_view();

		reference array = create_byte_array(descriptor.size());
		descriptor.copy_to(array_as_span<uint8>(array));

		return array;
	});
}

static reference method_type_create_method_descriptor_utf8(
	object& params_array, object& ret
) {
	return method_type_create_method_descriptor_utf8(
		array_as_span<reference>(params_array).transform_view(
			[](reference ref) -> _class& {
				return class_from_class_instance(ref);
			}
		),
		class_from_class_instance(ret)
	);
}

static void init_java_lang_invoke_method_type() {
	method_type_class = classes.find_or_load(
		c_string{ "java/lang/invoke/MethodType" }
	);

	method_type_parameter_types_instance_field_index = {
		(uint16) method_type_class->instance_fields().find_index_of(
			c_string{ "parameterTypes_" },
			c_string{ "[Ljava/lang/Class;" }
		)
	};

	method_type_return_type_instance_field_index = {
		(uint16) method_type_class->instance_fields().find_index_of(
			c_string{ "returnType_" },
			c_string{ "Ljava/lang/Class;" }
		)
	};

	method_type_descriptor_instance_field_index = {
		(uint16) method_type_class->instance_fields().find_index_of(
			c_string{ "descriptorUTF8_" },
			c_string{ "[B" }
		)
	};

	method_type_class->declared_methods().find(
		c_string{ "descriptorUTF8" },
		c_string{ "([Ljava/lang/Class;Ljava/lang/Class;)[B" }
	).native_function(
		(void*) (object*(*)(native_interface_environment*, object*, object*))
		[](native_interface_environment*, object* params_array, object* ret) {
			return &
				method_type_create_method_descriptor_utf8(*params_array, *ret)
				.unsafe_release_without_destroing();
		}
	);
}

static span<const char> method_type_descriptor(object& mt) {
	reference& utf8_desc = mt[
		method_type_descriptor_instance_field_index
	].get<reference>();
	return array_as_span<const char>(utf8_desc);
}