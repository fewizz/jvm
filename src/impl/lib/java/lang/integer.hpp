#include "decl/classes.hpp"
#include "decl/primitives.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/string.hpp"

#include <number.hpp>

static void init_java_lang_integer() {
	_class& integer_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Integer" }
	);

	integer_class.declared_methods().find(
		c_string{ "getPrimitiveClass" }, c_string{ "()Ljava/lang/Class;" }
	).native_function(
		(void*)+[](native_environment*) -> object* {
			return int_class->instance().object_ptr();
		}
	);

	integer_class.declared_methods().find(
		c_string{ "toString" }, c_string{ "(II)Ljava/lang/String;" }
	).native_function(
		(void*)+[](native_environment*, int32 value, int32 radix) -> object* {
			int32 max_radix = 10 + 26;
			int32 min_radix = 2;
			if(radix < min_radix || radix > max_radix) {
				radix = 10;
			}

			nuint digits_count = 0;
			bool negative = value < 0;

			number{ value }.for_each_digit(
				radix,
				[&](nuint){ ++digits_count; }
			);

			char digits_array[digits_count + negative];
			nuint digit_index = 0;
			if(negative) {
				digits_array[digit_index++] = '-';
			}

			number{ value }.for_each_digit(
				radix,
				[&](nuint digit) {
					digits_array[digit_index++] =
						digit < 10 ?
						'0' + digit :
						'a' + (digit - 10);
				}
			);

			span string_data{ digits_array, digits_count };
			expected<reference, reference> possible_string
				= try_create_string_from_utf8(string_data);

			if(possible_string.is_unexpected()) {
				thrown_in_native = possible_string.get_unexpected();
				return nullptr;
			}

			reference string = move(possible_string.get_expected());

			return & string.unsafe_release_without_destroing();
		}
	);

}