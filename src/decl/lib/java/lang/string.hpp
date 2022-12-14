#pragma once

#include "decl/class.hpp"
#include "decl/array.hpp"
#include "decl/object.hpp"

#include <unicode/utf8.hpp>
#include <unicode/utf16.hpp>

static optional<_class&> string_class{};
inline instance_field_position string_value_field_position;

template<typename Handler>
inline void for_each_string_codepoint(object& str, Handler&& handler) {
	reference& value = str.get<reference>(string_value_field_position);

	uint8* it = array_data<uint8>(value);
	int32 len = array_length(value);
	auto end = it + len * sizeof(uint16);

	while(it != end) {
		auto cp = utf16::decoder{}(it);
		if(cp.is_unexpected()) { abort(); }
		// TODO unexpected
		handler(cp.get_expected());
	}
}

inline nuint string_utf8_length(object& str);

template<typename Handler>
inline decltype(auto) view_string_on_stack_as_utf8(
	object& str, Handler&& handler
) {
	return view_on_stack<char>{ string_utf8_length(str) }(
		[&](span<char> utf8_str) {
			auto it = utf8_str.iterator();
			for_each_string_codepoint(str, [&](unicode::code_point cp) {
				utf8::encoder{}(cp, it);
			});
			return handler(utf8_str);
		}
	);
}

static inline reference create_string(span<uint16> data);

template<basic_range String>
static reference create_string_from_utf8(String&& str_utf8);