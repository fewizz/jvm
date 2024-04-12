#include "decl/lib/java/lang/string.hpp"

#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <range.hpp>

#include <posix/io.hpp>
#include <posix/memory.hpp>

#include <unicode/utf16.hpp>
#include <unicode/utf8.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_string(span<utf16::unit> data) {
	expected<reference, reference> possible_data_ref
		= try_create_char_array(data.size());
	
	if(possible_data_ref.is_unexpected()) {
		return unexpected{ possible_data_ref.move_unexpected() };
	}
	reference data_ref = possible_data_ref.move_expected();

	data.copy_to(array_as_span<utf16::unit>(data_ref));

	expected<reference, reference> possible_string_ref
		= try_create_object(j::string::c.get());
	
	if(possible_string_ref.is_unexpected()) {
		return unexpected{ possible_string_ref.move_unexpected() };
	}

	reference string_ref = possible_string_ref.move_expected();

	string_ref->set(j::string::value_field_position, move(data_ref));
	return string_ref;
}

static inline void init_java_lang_string() {
	j::string::c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/String"s
	);

	j::string::value_field_position = j::string::c->instance_field_position(
		u8"value_"s, u8"[C"s
	);

	j::string::c.get().declared_instance_methods().find(
		u8"startsWith"s,
		u8"(Ljava/lang/String;)Z"s
	).native_function(
		(void*)+[](native_environment*, j::string* ths, j::string* prefix)
		-> bool {
			if(prefix == nullptr) {
				return false;
			}

			nuint prefix_size = prefix->length_utf16_units();

			if(prefix_size > ths->length_utf16_units()) {
				return false;
			}

			return ths->as_utf16_units_span()
				.shrink_view(prefix_size)
				.has_equal_size_and_elements(
					prefix->as_utf16_units_span()
				);
		}
	);
}