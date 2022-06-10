#pragma once

#include "../field.hpp"
#include "class/file/descriptor/reader.hpp"
#include "class/file/field/reader.hpp"

#include <core/meta/elements/of.hpp>
#include <core/meta/elements/one_of.hpp>

template<typename Iterator>
static inline elements::of<
	class_file::field::reader<Iterator, class_file::field::reader_stage::end>,
	elements::one_of<field, static_field>
>
read_field(_class& c, class_file::field::reader<Iterator> access_reader) {
	auto [name_index_reader, access_flags] = access_reader();
	auto [descriptor_reader, name_index] = name_index_reader();
	auto [attributes_reader, descriptor_index] = descriptor_reader();
	auto end = attributes_reader(
		[&](auto name_index){ return c.utf8_constant(name_index); },
		[&]<typename Type>(Type) {}
	);

	field f {
		c, access_flags,
		::name_index{ name_index }, ::descriptor_index{ descriptor_index }
	};

	if(!access_flags.get(class_file::access_flag::_static))
		return { end, f };

	field_value fv;

	auto descriptor = c.utf8_constant(descriptor_index);

	using namespace class_file;

	bool result = descriptor::read_field(
		descriptor.begin(),
		[&]<typename DescriptorType>(DescriptorType) {
			return fv.set_default_value<DescriptorType>();
		}
	);

	if(!result) {
		fputs("couldn't read field descriptor", stderr);
		abort();
	}

	return { end, { f, fv } };
}