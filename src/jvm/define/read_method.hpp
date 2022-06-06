#pragma once

#include "../method.hpp"
#include "class/file/method/reader.hpp"

#include <core/meta/elements/of.hpp>
#include <core/meta/elements/one_of.hpp>

template<typename Iterator>
elements::of<
	class_file::method::reader<Iterator, class_file::method::reader_stage::end>,
	method
>
read_method(_class& c, class_file::method::reader<Iterator> read_access) {
	auto [read_name_index, access_flags] = read_access();
	auto [descriptor_reader, name_index] = read_name_index();
	auto [read_attributes, descriptor_index] = descriptor_reader();

	code code;

	auto end = read_attributes(
		[&](auto name_index){ return c.utf8_constant(name_index); },
		[&]<typename Type>(Type x) {
			if constexpr (Type::type == class_file::attribute::type::code) {
				auto [read_max_locals, max_stack] = x();
				auto [read_code, max_locals] = read_max_locals();
				auto src0 = read_code.iterator_;
				uint32 length = read<uint32, endianness::big>(src0);
				code = ::code {
					span<uint8, uint32> { read_code.iterator_, length },
					max_stack, max_locals
				};
			}
		}
	);

	return { end, { c, access_flags, name_index, descriptor_index, code } };
}