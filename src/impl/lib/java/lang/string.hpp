#include "lib/java/lang/string.hpp"

#include "decl/classes.hpp"

#include <range.hpp>

#include <posix/io.hpp>
#include <posix/memory.hpp>

#include <unicode/utf16.hpp>
#include <unicode/utf8.hpp>

static inline reference create_string(span<uint16> data) {
	reference data_ref = create_char_array(data.size());
	array_data(data_ref, data.iterator());
	array_length(data_ref, data.size());
	reference string_ref = create_object(string_class.get());
	string_ref->set(string_value_field_position, move(data_ref));
	return string_ref;
}

inline nuint string_utf8_length(object& str) {
	nuint utf8_length = 0;
	for_each_string_codepoint(str, [&](unicode::code_point cp) {
		utf8_length += utf8::encoder{}.units(cp);
	});
	return utf8_length;
}

static inline void init_java_lang_string() {
	string_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/String" }
	);

	string_value_field_position = string_class->instance_field_position(
		c_string{ "value_" }, c_string{ "[C" }
	);
}