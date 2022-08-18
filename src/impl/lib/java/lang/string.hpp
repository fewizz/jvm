#include "lib/java/lang/string.hpp"

#include "decl/classes.hpp"
#include "decl/alloc.hpp"

#include <range.hpp>
#include <unicode/utf16.hpp>
#include <unicode/utf8.hpp>

static inline reference create_string(span<uint16> data) {
	reference data_ref = create_char_array(data.size());
	array_data(data_ref.object(), data.elements_ptr());
	reference string_ref = create_object(string_class.value());
	string_ref->values()[string_value_index] = move(data_ref);
	return string_ref;
}

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

template<basic_range String>
static reference create_string_from_utf8(String&& str_utf8) {
	auto it  = range_iterator(str_utf8);
	auto end = range_sentinel(str_utf8);
	nuint units = 0;
	while(it != end) {
		auto result = utf8::decoder{}(it);
		if(result.is_unexpected()) {
			fputs("invalid sequence", stderr); abort();
		}
		auto cp = result.get_expected();
		units += utf16::encoder<endianness::big>{}.units(cp);
	}

	span<uint16> data = allocate_for<uint16>(units).cast<uint16>();
	uint16* data_it = data.iterator();
	it = range_iterator(str_utf8);
	while(it != end) {
		auto cp = utf8::decoder{}( it );
		// TODO unexpected
		utf16::encoder<endianness::big>{}(cp.get_expected(), (uint8*) data_it);
	}

	return create_string(data);
}

static inline void init_java_lang_string() {
	string_class = classes.find_or_load(c_string{ "java/lang/String" });
	string_value_index = string_class->instance_fields().find_index_of(
		c_string{ "value_" }, c_string{ "[C" }
	);
}