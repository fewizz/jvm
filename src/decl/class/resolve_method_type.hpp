#pragma once

#include "./resolve_class.hpp"
#include "primitives.hpp"
#include "reference.hpp"

#include <range.hpp>

/* To resolve an unresolved symbolic reference to a method type, it is as if
resolution occurs of unresolved symbolic references to classes and interfaces
(§5.4.3.1) whose names correspond to the types given in the method descriptor
(§4.3.3) */
template<basic_range Descriptor>
inline reference resolve_method_type(_class& d, Descriptor&& descriptor) {
	auto get_class = [&]<typename Type>(Type t) -> _class& {
		if constexpr(same_as<Type, class_file::v>) {
			return void_class.get();
		}
		if constexpr(same_as<Type, class_file::b>) {
			return byte_class.get();
		}
		if constexpr(same_as<Type, class_file::c>) {
			return char_class.get();
		}
		if constexpr(same_as<Type, class_file::d>) {
			return double_class.get();
		}
		if constexpr(same_as<Type, class_file::f>) {
			return float_class.get();
		}
		if constexpr(same_as<Type, class_file::i>) {
			return int_class.get();
		}
		if constexpr(same_as<Type, class_file::j>) {
			return long_class.get();
		}
		if constexpr(same_as<Type, class_file::s>) {
			return short_class.get();
		}
		if constexpr(same_as<Type, class_file::z>) {
			return bool_class.get();
		}
		else if constexpr(same_as<Type, class_file::object>) {
			return resolve_class(d, t);
		}
		else if constexpr(same_as<Type, class_file::array>) {
			return resolve_class(d, t);
		}
	};

	/* The result of successful method type resolution is a reference to an
	   instance of java.lang.invoke.MethodType which represents the method
	   descriptor. */
	if(descriptor.size() > 0 && descriptor[0] == '(') {
		class_file::method_descriptor::reader reader{ descriptor.iterator() };
		nuint parameters_count = reader.try_read_parameters_count(
			[](auto) { abort(); }
		).get();
		_class* params_classes[parameters_count];
		parameters_count = 0;
		_class* ret_class;
		
		class_file::method_descriptor::try_read_parameter_and_return_types(
			descriptor.iterator(),
			[&]<typename ParamType>(ParamType p) {
				params_classes[parameters_count++] = &get_class(p);
			},
			[&]<typename ReturnType>(ReturnType r) {
				ret_class = &get_class(r);
			},
			[](auto) { abort(); }
		);
		return create_method_type(
			span{ params_classes, parameters_count },
			*ret_class,
			descriptor
		);
	}
	else {
		//class_file::descriptor::read_field(descriptor.iterator(), resolve);
		abort();
	}


}