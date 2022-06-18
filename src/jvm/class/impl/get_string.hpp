#pragma once

#include "../../classes/find_or_load.hpp"
#include "../../class/declaration.hpp"
#include "../../object/create.hpp"
#include "../../../abort.hpp"

#include <core/transform.hpp>
#include <unicode/utf_8.hpp>
#include <unicode/utf_16.hpp>
#include <stdio.h>

reference _class::get_string(uint16 string_index) {
	if(auto& t = trampoline(string_index); !t.is<elements::none>()) {
		return t.get<reference>();
	}

	auto text_utf8 = utf8_constant(string_constant(string_index).string_index);

	nuint codepoints = 0;
	nuint utf16_units = 0;

	{
		auto it = text_utf8.begin();
		auto end = text_utf8.end();
		while(it != end) {
			auto result = utf_8::decoder{}(it);
			if(result.is_unexpected()) {
				fputs("invalid sequence", stderr); abort();
			}
			auto cp = result.get_expected();
			utf16_units += utf_16::encoder<endianness::big>{}.units(cp);
			++codepoints;
		}
	}

	auto data = (uint8*)malloc(utf16_units * sizeof(jchar));

	{
		auto data_it = data;
		auto it = text_utf8.begin();
		auto end = text_utf8.end();
		while(it != end) {
			auto cp = utf_8::decoder{}(it).get_expected();
			utf_16::encoder<endianness::big>{}(cp, data_it);
		}
	}

	_class& array_class = find_or_load_class(c_string{ "byte[]" });
	auto data_ref = create_object(array_class);
	data_ref.object().values()[0] = field_value {
		jlong { (int64) data }
	};
	data_ref.object().values()[1] =
		jint{ (int32) (utf16_units * sizeof(jchar)) };

	_class& string_class = find_or_load_class(c_string{"java/lang/String"});
	auto string_ref = create_object(string_class);

	auto value_index0 = string_class.try_find_instance_field_index(
		c_string{ "value" },
		c_string{ "[B" }
	);
	if(!value_index0.has_value()) {
		fputs("couldn't find 'value' field in 'String'", stderr); abort();
	}
	auto value_index = value_index0.value();

	auto coder_index0 = string_class.try_find_instance_field_index(
		c_string{ "coder" },
		c_string{ "B" }
	);
	auto coder_index = coder_index0.value();

	string_ref.object().values()[value_index] = data_ref;
	string_ref.object().values()[coder_index] = field_value {
		jbyte{ 1 } // UTF16
	};

	trampoline(string_index) = string_ref;

	return string_ref;
}