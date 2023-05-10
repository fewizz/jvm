#include "lib/java/lang/string.hpp"

#include "decl/classes.hpp"

#include <range.hpp>

#include <posix/io.hpp>
#include <posix/memory.hpp>

#include <unicode/utf16.hpp>
#include <unicode/utf8.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_string(span<uint16> data) {
	expected<reference, reference> possible_data_ref
		= try_create_char_array(data.size());
	
	if(possible_data_ref.is_unexpected()) {
		return unexpected{ possible_data_ref.move_unexpected() };
	}
	reference data_ref = possible_data_ref.move_expected();

	array_data(data_ref, data.iterator());
	array_length(data_ref, data.size());

	expected<reference, reference> possible_string_ref
		= try_create_object(string_class.get());
	
	if(possible_string_ref.is_unexpected()) {
		return unexpected{ possible_string_ref.move_unexpected() };
	}

	reference string_ref = possible_string_ref.move_expected();

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
		c_string{ u8"java/lang/String" }
	);

	string_value_field_position = string_class->instance_field_position(
		c_string{ u8"value_" }, c_string{ u8"[C" }
	);
}