#pragma once

#include "declaration.hpp"
#include "../class/declaration.hpp"
#include "../field/declaration.hpp"
#include "class/file/descriptor/reader.hpp"

object::object(::_class& c) :
	class_{ c },
	values_{ c.instance_fields().size() }
{
	for(field& f : c.instance_fields()) {
		using namespace class_file;

		field_value fv;

		bool result = descriptor::read_field(
			f.descriptor().begin(),
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
	}
}

inline void object::on_reference_removed() {
	--references_;
	if(references_ == 0) {
		this->~object();
		free(this);
	}
}

inline field_value& object::operator [] (field_index index) {
	return values_[(uint16) index];
}