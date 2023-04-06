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
inline expected<reference, reference>
try_resolve_method_type(_class& d, Descriptor&& descriptor) {
	auto get_class = [&]<typename Type>(Type t)
	-> expected<_class&, reference> {
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
			return try_resolve_class(d, t);
		}
		else if constexpr(same_as<Type, class_file::array>) {
			return try_resolve_class(d, t);
		}
	};

	/* The result of successful method type resolution is a reference to an
	   instance of java.lang.invoke.MethodType which represents the method
	   descriptor. */
	if(descriptor.size() > 0 && descriptor[0] == '(') {
		class_file::method_descriptor::reader reader{ descriptor.iterator() };
		nuint parameters_count = reader.try_read_parameters_count(
			[](auto) { posix::abort(); }
		).get();
		_class* params_classes[parameters_count];
		parameters_count = 0;
		_class* ret_class;

		reference thrown{};
		class_file::method_descriptor::try_read_parameter_and_return_types(
			descriptor.iterator(),
			[&]<typename ParamType>(ParamType p) {
				expected<_class&, reference> possible_c = get_class(p);
				if(possible_c.is_unexpected()) {
					thrown = move(possible_c.get_unexpected());
					return;
				}
				_class& c = possible_c.get_expected();
				params_classes[parameters_count++] = &c;
			},
			[&]<typename ReturnType>(ReturnType r) {
				if(!thrown.is_null()) return;
				expected<_class&, reference> possible_c = get_class(r);
				if(possible_c.is_unexpected()) {
					thrown = move(possible_c.get_unexpected());
					return;
				}
				_class& c = possible_c.get_expected();
				ret_class = &c;
			},
			[](auto) { posix::abort(); }
		);
		return try_create_method_type(
			*ret_class,
			span<_class&>{ params_classes, parameters_count },
			descriptor
		);
	}
	else {
		//class_file::descriptor::read_field(descriptor.iterator(), resolve);
		posix::abort();
	}


}