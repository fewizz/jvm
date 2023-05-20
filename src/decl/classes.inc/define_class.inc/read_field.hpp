#pragma once

#include "decl/class/constants.hpp"
#include "decl/field.hpp"

#include <class_file/field_reader.hpp>

template<basic_iterator Iterator>
tuple<field, Iterator> read_field(
	constants& const_pool,
	class_file::field::reader<Iterator> field_reader
) {
	auto [access_flags, name_index_reader] {
		field_reader.read_access_flags_and_get_name_index_reader()
	};
	auto [name_index, descriptor_index_reader] {
		name_index_reader.read_and_get_descriptor_index_reader()
	};
	auto [descriptor_index, attributes_reader] {
		descriptor_index_reader.read_and_get_attributes_reader()
	};
	auto it = attributes_reader.read_and_get_advanced_iterator(
		[&](auto name_index) {
			return const_pool.utf8_constant(name_index);
		},
		[&]<typename Type>(Type) {
		}
	);
	class_file::constant::utf8 name =
		const_pool.utf8_constant(name_index);
	class_file::constant::utf8 descriptor =
		const_pool.utf8_constant(descriptor_index);

	return {
		{ access_flags, name, descriptor },
		it
	};
}