#pragma once

#include "decl/class.hpp"
#include "decl/array.hpp"
#include "decl/object.hpp"

#include <unicode/utf8.hpp>
#include <unicode/utf16.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_string(span<utf16::unit> data);

template<basic_range String>
[[nodiscard]] expected<reference, reference>
try_create_string_from_utf8(String&& str_utf8) {
	auto it  = range_iterator(str_utf8);
	auto end = range_sentinel(str_utf8);
	nuint units = 0;
	utf8::decoder d0{it};
	while(it != end) {
		auto result = d0.read();
		if(result.is_unexpected()) {
			print::err("invalid sequence\n");
			posix::abort();
		}
		auto cp = result.get_expected();
		units += utf16::encoder{}.units(cp);
	}

	auto data = initialised{ posix::allocate<utf16::unit>(units) };
	uint8* data_it = (uint8*) data.iterator();
	it = range_iterator(str_utf8);
	utf8::decoder d1 = {it};
	while(it != end) {
		auto cp = d1.read();
		utf16::encoder{}(cp.get_expected(), data_it);
	}

	return try_create_string(data);
}

template<basic_range String>
[[nodiscard]] reference create_string_from_utf8(String&& str_utf8) {
	expected<reference, reference> possible_str
		= try_create_string_from_utf8(forward<String>(str_utf8));
	if(possible_str.is_unexpected()) {
		posix::abort();
	}
	return possible_str.move_expected();
}

namespace j {

struct string : object {
	using object::object;

	inline static optional<::c&> c{};
	inline static layout::position value_field_position;

	template<typename Handler>
	void for_each_codepoint(Handler&& handler) {
		reference& value = get<reference>(value_field_position);

		uint8* it = array_data<uint8>(value);
		int32 len = array_length(value);
		auto end = it + len * sizeof(uint16);

		while(it != end) {
			auto cp = utf16::decoder{}(it);
			if(cp.is_unexpected()) { posix::abort(); }
			// TODO unexpected
			handler(cp.get_expected());
		}
	}

	template<typename Handler>
	void for_each_utf16_unit(Handler&& handler) {
		reference& value = get<reference>(value_field_position);
		for(utf16::unit unit : array_as_span<utf16::unit>(value)) {
			handler(unit);
		}
	}

	nuint length_utf8() {
		nuint len = 0;
		for_each_codepoint([&](unicode::code_point cp) {
			len += utf8::encoder{}.units(cp);
		});
		return len;
	}

	nuint length_utf16_units() {
		reference& value = get<reference>(value_field_position);
		return array_length(value);
	}

	template<typename Handler>
	decltype(auto) view_on_stack_as_utf8(Handler&& handler) {
		return view_on_stack<utf8::unit>{ length_utf8() }(
			[&](span<utf8::unit> utf8_str) -> decltype(auto) {
				auto it = utf8_str.iterator();
				for_each_codepoint([&](unicode::code_point cp) {
					utf8::encoder{}(cp, it);
				});
				return handler(utf8_str);
			}
		);
	}

	span<utf16::unit> as_utf16_units_span() {
		reference& value = get<reference>(value_field_position);
		return array_as_span<utf16::unit>(value);
	}

};

}