#pragma once

#include "array.hpp"
#include "classes/find_or_load.hpp"
#include "class/decl.hpp"
#include "object/create.hpp"
#include "lib/java_lang/string.hpp"
#include "abort.hpp"

#include <unicode/utf8.hpp>
#include <unicode/utf16.hpp>

#include <core/transform.hpp>

#include <stdio.h>

inline reference _class::get_string(uint16 string_index) {
	if(auto& t = trampoline(string_index); !t.is<elements::none>()) {
		if(!t.is<::reference>()) {
			fputs("invalid const pool entry", stderr);
			abort();
		}
		return t.get<::reference>();
	}

	namespace cc = class_file::constant;

	cc::string string = string_constant(string_index);
	cc::utf8 text_utf8 = utf8_constant(string.string_index);

	::reference utf16_string_ref {
		create_string_from_utf8(text_utf8)
	};

	trampoline(string_index) = utf16_string_ref;

	return utf16_string_ref;
}