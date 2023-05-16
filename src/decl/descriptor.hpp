#pragma once

#include <class_file/descriptor/type.hpp>

#include <variant.hpp>

using one_of_descriptor_return_types = variant<
	class_file::z, class_file::b,
	class_file::c, class_file::s,
	class_file::i, class_file::f,
	class_file::j, class_file::d,
	class_file::v,
	class_file::array, class_file::object
>;

using one_of_descriptor_parameter_types = variant<
	class_file::z, class_file::b,
	class_file::c, class_file::s,
	class_file::i, class_file::f,
	class_file::j, class_file::d,
	class_file::array, class_file::object
>;

using one_of_descriptor_field_types = one_of_descriptor_parameter_types;

template<class_file::descriptor_type Type>
static constexpr nuint descriptor_type_stack_size =
	same_as_any<Type, class_file::j, class_file::d> ?
	2 :
	1;

