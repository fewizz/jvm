#pragma once

#include "field/decl.hpp"
#include "field/value.hpp"
#include "class/const_pool.hpp"
#include "abort.hpp"

#include <class/file/descriptor/reader.hpp>

#include <stdio.h>

struct static_field : field {
private:
	field_value value_;
public:
	static_field(field f, const_pool& const_pool):
		field{ f }
	{
		using namespace class_file;
		constant::utf8 descriptor {
			const_pool.utf8_constant(descriptor_index())
		};

		bool result = descriptor::read_field(
			descriptor.begin(),
			[&]<typename DescriptorType>(DescriptorType) {
				return value_.set_default_value<DescriptorType>();
			}
		);

		if(!result) {
			fputs("couldn't set static field's default value", stderr);
			abort();
		}
	};

	auto& value() { return value_; }
};