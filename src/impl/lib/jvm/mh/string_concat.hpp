#include "decl/lib/jvm/mh/string_concat.hpp"

#include "decl/classes.hpp"

static void init_jvm_mh_string_concat() {
	jvm::string_concat::c = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"jvm/mh/StringConcat" }
	);

	jvm::string_concat::constructor
		= jvm::string_concat::c->declared_instance_methods().find(
			c_string{ u8"<init>" },
			c_string{ u8"(Ljava/lang/invoke/MethodType;Ljava/lang/String;)V" }
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
			j::method_handle& ths0
		) -> optional<reference> {
			jvm::string_concat& ths = (jvm::string_concat&) ths0;
			j::string& recipe = ths.recipe();
			
			nuint size = 0;

			nuint beginning
				= stack.size() - ths.method_type().compute_args_stack_size();
			nuint arg_index = 0;
			nuint arg_stack_beginning = beginning;

			recipe.for_each_utf16_unit([&](utf16::unit unit) {
				if(unit == 1) {
					ths.method_type().parameter_types_view()[arg_index++]
					.view_non_void_raw_type([&]<typename Type> {
						if constexpr(same_as<Type, reference>) {
							reference& ref = stack.get<reference>(
								arg_stack_beginning
							);
							if(ref.c().is_not(j::string::c.get())) {
								posix::abort();
							}
							j::string& str = (j::string&) ref.object();
							size += str.length_utf16_units();
							++arg_stack_beginning;
						}
						else if constexpr(same_as_any<Type, int32, int64>) {
							Type val = stack.get<Type>(arg_stack_beginning);
							if(val < 0) {
								++size;
							}
							number{ val }.for_each_digit(10, [&](auto) {
								++size;
							});
							++arg_stack_beginning;
						}
						else {
							posix::abort();
						}
					});
					return;
				}
				if(unit == 2) {
					posix::abort();
					return;
				}
				size += 1;
			});

			utf16::unit string_data_raw[size];
			span<utf16::unit> string_data{ string_data_raw, size };
			nuint written = 0;
			arg_index = 0;
			arg_stack_beginning = beginning;

			recipe.for_each_utf16_unit([&](utf16::unit unit) {
				if(unit == 1) {
					ths.method_type().parameter_types_view()[arg_index++]
					.view_non_void_raw_type([&]<typename Type> {
						if constexpr(same_as<Type, reference>) {
							reference& ref = stack.get<reference>(
								arg_stack_beginning
							);
							if(ref.c().is_not(j::string::c.get())) {
								posix::abort();
							}
							j::string& str = (j::string&) ref.object();
							str.as_utf16_units_span().copy_to(
								iterator_and_sentinel {
									string_data_raw + written,
									string_data_raw + size
								}.as_range()
							);
							written += str.as_utf16_units_span().size();
							++arg_stack_beginning;
						}
						else if constexpr(same_as_any<Type, int32, int64>) {
							Type val = stack.get<Type>(arg_stack_beginning);
							if(val < 0) {
								string_data_raw[written++] = '-';
							}
							number{ val }.for_each_digit(10, [&](auto digit) {
								string_data_raw[written++] = '0' + digit;
							});
							++arg_stack_beginning;
						}
						else {
							posix::abort();
						}
					});
					return;
				}
				if(unit == 2) {
					posix::abort();
					return;
				}

				string_data[written++] = unit;
			});

			stack.erase_back_until(beginning);
			expected<reference, reference> possible_resulting_string
				= try_create_string(string_data);
			
			if(possible_resulting_string.is_unexpected()) {
				return possible_resulting_string.move_unexpected();
			}

			stack.emplace_back(possible_resulting_string.move_expected());

			return {};
		}
	);
}