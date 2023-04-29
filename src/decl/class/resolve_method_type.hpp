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
[[nodiscard]] inline expected<reference, reference>
try_resolve_method_type(c& d, Descriptor&& descriptor) {
	bool method_descriptor = descriptor.size() > 0 && descriptor[0] == '(';
	if(!method_descriptor) {
		posix::abort();
	}
	class_file::method_descriptor::reader reader{ descriptor.iterator() };
	nuint parameters_count = reader.try_read_parameters_count(
		[](auto) { posix::abort(); }
	).get();
	c* params_classes[parameters_count];
	parameters_count = 0;
	c* ret_class;

	reference thrown{};

	class_file::method_descriptor::try_read_parameter_and_return_types(
		descriptor.iterator(),
		[&]<typename ParamType>(ParamType p) {
			expected<c&, reference> possible_c
				= try_resolve_class_from_type(d, p);
			if(possible_c.is_unexpected()) {
				thrown = possible_c.move_unexpected();
				return;
			}
			c& c = possible_c.get_expected();
			params_classes[parameters_count++] = &c;
		},
		[&]<typename ReturnType>(ReturnType r) {
			if(!thrown.is_null()) return;
			expected<c&, reference> possible_c
				= try_resolve_class_from_type(d, r);
			if(possible_c.is_unexpected()) {
				thrown = possible_c.move_unexpected();
				return;
			}
			c& c = possible_c.get_expected();
			ret_class = &c;
		},
		[](auto) { posix::abort(); }
	);

	/* Any exception that can be thrown as a result of failure of resolution
		of a class reference can thus be thrown as a result of failure of
		method type resolution. */
	if(!thrown.is_null()) {
		return unexpected{ move(thrown) };
	}

	/* The result of successful method type resolution is a reference to an
		instance of java.lang.invoke.MethodType which represents the method
		descriptor. */
	return try_create_method_type(
		*ret_class,
		span<c&>{ params_classes, parameters_count },
		descriptor
	);
}