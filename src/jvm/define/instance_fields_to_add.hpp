#pragma once

#include "class/file/access_flag.hpp"
#include "../name_index.hpp"
#include "../descriptor_index.hpp"

#include <core/range_of_value_type_same_as.hpp>
#include <core/meta/type/is_same_as.hpp>
#include <core/meta/elements/of.hpp>

using field_constructor_args = elements::of<
	class_file::access_flags, name_index, descriptor_index
>;

template<range_of<field_constructor_args> FieldsRange>
class instance_fields_to_add {
	FieldsRange fields_;

public:

	instance_fields_to_add(FieldsRange&& range) :
		fields_{ forward<FieldsRange>(range) }
	{}

	decltype(auto) fields() {
		return (fields_);
	}

};

template<range_of<field_constructor_args> FieldsRange>
instance_fields_to_add(FieldsRange&&) -> instance_fields_to_add<FieldsRange>;

struct is_instance_fields_to_add : type::predicate_marker {

	template<typename Type>
	static constexpr bool for_type = false;

	template<typename FieldsRange>
	static constexpr bool for_type<instance_fields_to_add<FieldsRange>> = true;

};