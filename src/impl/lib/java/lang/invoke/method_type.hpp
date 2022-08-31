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

static reference method_type_method_descriptor(
	object& params_array, object& ret
) {
	span<reference> params = array_as_span<reference>(params_array);

	auto desc_class_name = [](_class& c) {
		if(c.is_primitive()) {
			if(&c == void_class.ptr())   { return span{ c_string{ "V" } }; }
			if(&c == bool_class.ptr())   { return span{ c_string{ "Z" } }; }
			if(&c == byte_class.ptr())   { return span{ c_string{ "B" } }; }
			if(&c == short_class.ptr())  { return span{ c_string{ "S" } }; }
			if(&c == char_class.ptr())   { return span{ c_string{ "C" } }; }
			if(&c == int_class.ptr())    { return span{ c_string{ "I" } }; }
			if(&c == float_class.ptr())  { return span{ c_string{ "F" } }; }
			if(&c == long_class.ptr())   { return span{ c_string{ "J" } }; }
			if(&c == double_class.ptr()) { return span{ c_string{ "D" } }; }
			abort();
		}
		return span{ c.name() };
	};

	auto descriptor = ranges {
		array{ '(' },
		range {
			range{ params }.transform_view([&](reference& param_ref) {
				_class& c = class_from_class_instance(param_ref.object());
				return desc_class_name(c);
			})
		}.flat_view(),
		array{ ')' },
		desc_class_name(class_from_class_instance(ret))
	}.concat_view();

	nuint size = iterator_and_sentinel{
		range_iterator(descriptor), range_sentinel(descriptor)
	}.distance();

	reference array = create_byte_array(size);
	range{ descriptor }.copy_to(array_as_span<uint8>(array.object()));

	return array;
}

static void init_java_lang_invoke_method_type() {
	method_type_class = classes.find_or_load(
		c_string{ "java/lang/invoke/MethodType" }
	);

	method_type_parameter_types_instance_field_index = {
		method_type_class->instance_fields().find_index_of(
			c_string{ "parameterTypes_" },
			c_string{ "[Ljava/lang/Class;" }
		)
	};

	method_type_return_type_instance_field_index = {
		method_type_class->instance_fields().find_index_of(
			c_string{ "returnType_" },
			c_string{ "Ljava/lang/Class;" }
		)
	};

	method_type_descriptor_instance_field_index = {
		method_type_class->instance_fields().find_index_of(
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
				method_type_method_descriptor(*params_array, *ret)
				.unsafe_release_without_destroing();
		}
	);
}

static span<const char> method_type_descriptor(object& mt) {
	reference& utf8_desc = mt.values()[
		method_type_descriptor_instance_field_index
	].get<reference>();
	return array_as_span<const char>(utf8_desc.object());
}