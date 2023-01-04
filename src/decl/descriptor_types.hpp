#pragma once

#include "reference.hpp"
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