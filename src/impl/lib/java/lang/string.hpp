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
	reference string_ref = create_object(string_class.value());
	string_ref->values()[string_value_index] = move(data_ref);
	return string_ref;
}

inline nuint string_utf8_length(object& str) {
	nuint utf8_length = 0;
	for_each_string_codepoint(str, [&](unicode::code_point cp) {
		utf8_length += utf8::encoder{}.units(cp);
	});
	return utf8_length;
}

template<basic_range String>
static reference create_string_from_utf8(String&& str_utf8) {
	auto it  = range_iterator(str_utf8);
	auto end = range_sentinel(str_utf8);
	nuint units = 0;
	while(it != end) {
		auto result = utf8::decoder{}(it);
		if(result.is_unexpected()) {
			posix::std_err().write_from(c_string{ "invalid sequence" });
		}
		auto cp = result.get_expected();
		units += utf16::encoder<endianness::big>{}.units(cp);
	}

	span<uint16> data =
		posix::allocate_non_owning_memory_of<uint16>(units).cast<uint16>();
	uint8* data_it = (uint8*) data.iterator();
	it = range_iterator(str_utf8);
	while(it != end) {
		auto cp = utf8::decoder{}( it );
		// TODO unexpected
		utf16::encoder<endianness::big>{}(cp.get_expected(), data_it);
	}

	return create_string(data);
}

static inline void init_java_lang_string() {
	string_class = classes.find_or_load(c_string{ "java/lang/String" });
	string_value_index = string_class->instance_fields().find_index_of(
		c_string{ "value_" }, c_string{ "[C" }
	);
}