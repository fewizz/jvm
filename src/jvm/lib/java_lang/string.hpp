#pragma once

#include "../../class/declaration.hpp"
#include "../../classes/find_or_load.hpp"
#include "../../array.hpp"
#include "../../object/create.hpp"

#include <core/meta/elements/optional.hpp>
#include <unicode/utf_16.hpp>
#include <unicode/utf_8.hpp>

static optional<_class&> string_class{};
static instance_field_index string_value_index{};
static instance_field_index string_coder_index{};

enum class string_coder {
	latin_1 = 0, utf16 = 1
};

template<typename Handler>
inline void for_each_string_codepoint(object& str, Handler&& handler) {
	reference& values_ref = str.values()[string_value_index].get<reference>();
	object& values = values_ref.object();

	uint8* it = array_data<uint8>(values);
	int32 len = array_length(values);
	auto end = it + len;

	while(it != end) {
		unicode::code_point cp = utf_16::decoder<endianness::big>{}(it);
		handler(cp);
	}
}

static inline reference create_string(span<uint8> data, string_coder coder) {
	reference data_ref = create_object(byte_array_class.value());
	array_data(data_ref.object(), data.data());
	array_length(data_ref.object(), data.size());

	reference ref = create_object(string_class.value());

	ref.object().values()[string_value_index] = move(data_ref);
	ref.object().values()[string_coder_index] = jbyte{ (uint8) coder };

	return ref;
}

template<range String>
static reference
create_string_from_utf8(String&& str_utf8, string_coder coder) {
	auto it = str_utf8.begin();
	auto end = str_utf8.end();
	nuint utf16_units = 0;
	while(it != end) {
		auto result = utf_8::decoder{}(it);
		if(result.is_unexpected()) {
			fputs("invalid sequence", stderr); abort();
		}
		auto cp = result.get_expected();
		utf16_units += utf_16::encoder<endianness::big>{}.units(cp);
	}

	nuint data_len = utf16_units * sizeof(uint16);
	uint8* data = (uint8*) malloc(data_len);
	auto data_it = data;
	it = str_utf8.begin();
	while(it != end) {
		auto cp = utf_8::decoder{}(it).get_expected();
		utf_16::encoder<endianness::big>{}(cp, data_it);
	}

	return create_string(span{ data, data_len }, coder);
}

static inline void init_java_lang_string() {

	string_class = find_or_load_class(c_string{ "java/lang/String" });
	string_value_index = string_class->find_instance_field_index(
		c_string{ "value" }, c_string{ "[B" }
	);
	string_coder_index = string_class->find_instance_field_index(
		c_string{ "coder" }, c_string{ "B" }
	);

}