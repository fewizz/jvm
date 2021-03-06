#pragma once

#include "class/decl.hpp"
#include "classes/load.hpp"
#include "array.hpp"
#include "object/create.hpp"

#include <unicode/utf16.hpp>
#include <unicode/utf8.hpp>

#include <core/meta/elements/optional.hpp>

static optional<_class&> string_class{};
static instance_field_index string_value_index{};

template<typename Handler>
inline void for_each_string_codepoint(object& str, Handler&& handler) {
	reference& value_ref = str.values()[string_value_index].get<reference>();
	object& values = value_ref.object();

	uint16* it = array_data<uint16>(values);
	int32 len = array_length(values);
	auto end = it + len;

	while(it != end) {
		auto cp = utf16::decoder<endianness::big>{}((uint16*) it);
		// TODO unexpected
		handler(cp.get_expected());
	}
}

static inline reference create_string(span<uint16> data) {
	reference data_ref = create_char_array(data.size());
	array_data(data_ref.object(), data.data());
	reference string_ref = create_object(string_class.value());
	string_ref.object().values()[string_value_index] = move(data_ref);
	return string_ref;
}

template<range String>
static reference
create_string_from_utf8(String&& str_utf8) {
	auto it = str_utf8.begin();
	auto end = str_utf8.end();
	nuint units = 0;
	while(it != end) {
		auto result = utf8::decoder{}(it);
		if(result.is_unexpected()) {
			fputs("invalid sequence", stderr); abort();
		}
		auto cp = result.get_expected();
		units += utf16::encoder<endianness::big>{}.units(cp);
	}

	nuint data_len = units * sizeof(uint16);
	uint16* data = (uint16*) malloc(data_len);
	uint16* data_it = data;
	it = str_utf8.begin();
	while(it != end) {
		auto cp = utf8::decoder{}( it );
		// TODO unexpected
		utf16::encoder<endianness::big>{}(cp.get_expected(), (uint8*) data_it);
	}

	return create_string(span{ data, units });
}

static inline void init_java_lang_string() {

	string_class = load_class(c_string{ "java/lang/String" });
	string_value_index = string_class->find_instance_field_index(
		c_string{ "value_" }, c_string{ "[C" }
	);

}