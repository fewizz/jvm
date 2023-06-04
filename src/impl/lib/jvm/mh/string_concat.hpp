#include "decl/lib/jvm/mh/string_concat.hpp"

#include "decl/classes.hpp"

static void init_jvm_mh_string_concat() {
	jvm::string_concat::c = classes.load_non_array_class_by_bootstrap_class_loader(
		c_string{"jvm/mh/StringConcat"}
	);

	jvm::string_concat::constructor
		= jvm::string_concat::c->declared_instance_methods().find(
			c_string{ u8"<init>" },
			c_string{ u8"(Ljava/lang/invoke/MethodType;Ljava/lang/String;S)V" }
		);

	jvm::string_concat::recipe_field_position
		= jvm::string_concat::c->instance_field_position(
			c_string{ u8"recipe_" },
			c_string{ u8"Ljava/lang/String;" }
		);
	
	jvm::string_concat::c->declared_instance_methods().find(
		c_string{ u8"invokeExactPtr" }, c_string{ u8"()V" }
	).native_function(
		(void*)+[](
			jvm::string_concat& ths
		) -> optional<reference> {
			j::string& recipe = ths.recipe();
			
			nuint size = 0;

			nuint beginning = stack.size() - ths.compute_args_stack_size();
			nuint arg_index = beginning;

			recipe.for_each_utf16_unit([&](uint16 unit) {
				if(unit == 1) {
					reference& ref = stack.get<reference>(arg_index++);
					if(ref.c().is_not(j::string::c.get())) {
						posix::abort();
					}
					j::string& str = (j::string&) ref.object();
					size += str.length_utf16();
					return;
				}
				if(unit == 2) {
					posix::abort();
					return;
				}

				size += 1;
			});

			uint16 string_data_raw[size];
			span<uint16> string_data{ string_data_raw, size };
			nuint written = 0;
			arg_index = beginning;

			recipe.for_each_utf16_unit([&](uint16 unit) {
				if(unit == 1) {
					reference& ref = stack.get<reference>(arg_index++);
					if(ref.c().is_not(j::string::c.get())) {
						posix::abort();
					}
					j::string& str = (j::string&) ref.object();
					
					str.as_span_utf16().copy_to(
						iterator_and_sentinel {
							string_data_raw + written,
							string_data_raw + size
						}.as_range()
					);

					written += str.as_span_utf16().size();

					return;
				}
				if(unit == 2) {
					posix::abort();
					return;
				}

				string_data[written++] = unit;
			});

			stack.pop_back_until(beginning);
			expected<reference, reference> possible_resulting_string
				= try_create_string(string_data);
			
			if(possible_resulting_string.is_unexpected()) {
				return possible_resulting_string.move_unexpected();
			}

			return possible_resulting_string.move_expected();
		}
	);
}