#pragma once

#include "decl/class.hpp"
#include "decl/array.hpp"
#include "decl/object.hpp"

#include <unicode/utf8.hpp>
#include <unicode/utf16.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_string(span<uint16> data);

template<basic_range String>
[[nodiscard]] expected<reference, reference>
try_create_string_from_utf8(String&& str_utf8) {
	auto it  = range_iterator(str_utf8);
	auto end = range_sentinel(str_utf8);
	nuint units = 0;
	while(it != end) {
		auto result = utf8::decoder{}(it);
		if(result.is_unexpected()) {
			print::err("invalid sequence\n");
			posix::abort();
		}
		auto cp = result.get_expected();
		units += utf16::encoder{}.units(cp);
	}

	auto data = posix::allocate_raw<sizeof(uint16), alignof(uint16)>(units);
	uint8* data_it = (uint8*) data.iterator();
	it = range_iterator(str_utf8);
	while(it != end) {
		auto cp = utf8::decoder{}( it );
		utf16::encoder{}(cp.get_expected(), data_it);
	}

	return try_create_string(data.cast<uint16>());
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
		for(uint16 unit : array_as_span<uint16>(value)) {
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

	nuint length_utf16() {
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

	span<uint16> as_span_utf16() {
		reference& value = get<reference>(value_field_position);
		return array_as_span<uint16>(value);
	}

};

}