#pragma once

#include "object.hpp"

#include "object/reference/impl.hpp"
#include "class.hpp"
#include "field.hpp"

#include <class_file/descriptor/reader.hpp>

inline object::object(::_class& c) :
	class_{ c },
	values_{ c.instance_fields_count() }
{
	c.for_each_instance_field([&](instance_field_with_class fwc) {
		using namespace class_file;

		field_value fv;

		bool result = descriptor::read_field(
			fwc.descriptor().begin(),
			[&]<typename DescriptorType>(DescriptorType) {
				return fv.set_default_value<DescriptorType>();
			}
		);

		if(!result) {
			fputs(
				"couldn't read field descriptor while creating object",
				stderr
			);
			abort();
		}

		values_.emplace_back(move(fv));
	});
}

inline void object::on_reference_removed() {
	--references_;
	if(references_ == 0) {
		this->~object();
		free(this);
	}
}

inline void object::unsafe_decrease_reference_count_without_destroing() {
	--references_;
}

inline field_value& object::operator [] (instance_field_index index) {
	return values_[(uint16) index];
}