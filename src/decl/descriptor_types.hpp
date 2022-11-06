#pragma once

#include "reference.hpp"
#include <class_file/descriptor/type.hpp>

#include <variant.hpp>

using one_of_descriptor_types = variant<
	class_file::z, class_file::b,
	class_file::c, class_file::s,
	class_file::i, class_file::f,
	class_file::j, class_file::d,
	class_file::v,
	class_file::array, class_file::object
>;

using one_of_non_void_descriptor_types = variant<
	class_file::z, class_file::b,
	class_file::c, class_file::s,
	class_file::i, class_file::f,
	class_file::j, class_file::d,
	class_file::array, class_file::object
>;

using one_of_variable_types = variant<
	reference,
	int64, double,
	int32, float,
	int16, uint16,
	bool, int8
>;

using one_of_return_types = variant<
	void,
	reference,
	int64, double,
	int32, float,
	int16, uint16,
	bool, int8
>;