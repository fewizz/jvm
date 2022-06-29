#pragma once

#include "../../class/declaration.hpp"
#include "../../classes/find_or_load.hpp"
#include "../../array.hpp"

#include <core/meta/elements/optional.hpp>
#include <unicode/utf_16.hpp>

static optional<_class&> string_class{};
static instance_field_index string_value_index{};

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

static inline void init_java_lang_string() {

	string_class = find_or_load_class(c_string{ "java/lang/String" });
	string_value_index = string_class.value().find_instance_field_index(
		c_string{ "value" }, c_string{ "[B" }
	);

}