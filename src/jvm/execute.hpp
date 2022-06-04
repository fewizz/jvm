#pragma once

#include "objects.hpp"

struct stack_entry : elements::one_of<jvoid, int32, float, reference> {
	using base_type = elements::one_of<jvoid, int32, float, reference>;
	using base_type::base_type;

	stack_entry() : base_type{ jvoid{} } {}
};

inline stack_entry execute(method& m, span<stack_entry, uint16> args = {});

#include "class/file/reader.hpp"
#include "class/file/descriptor/reader.hpp"
#include "../abort.hpp"

#include <stdio.h>
#include <core/number.hpp>

template<range Name>
_class& find_or_load(Name name);

inline static constexpr bool info = true;
inline static nuint tab = 0;

inline void tabs() {
	for(nuint i = 1; i < tab; ++i) fputc('\t', stderr);
}

stack_entry execute(method& m, span<stack_entry, uint16> args) {
	_class& c = m._class();

	if(info) {
		++tab;
		tabs();
		fwrite(c.name().data(), 1, c.name().size(), stderr);
		fputc('.', stderr);
		fwrite(m.name().data(), 1, m.name().size(), stderr);
		fputc('(', stderr);
		fputc(')', stderr);
		fputc('\n', stderr);
	}

	if(m.code().data() == nullptr) {
		fputs("no code", stderr); abort();
	}

	class_file::attribute::code::reader<
		uint8*,
		class_file::attribute::code::reader_stage::code
	> reader{ m.code().data() };

	stack_entry stack[m.code().max_stack];
	nuint stack_size = 0;

	stack_entry local[m.code().max_locals];
	for(int i = 0; i < args.size(); ++i) local[i] = move(args[i]);

	using namespace class_file::code::instruction;
	namespace cc = class_file::constant;

	auto get = [&](field_value& value) {
		value.view([&]<typename ValueType>(ValueType& value) {
			if constexpr(same_as<reference, ValueType>) {
				stack[stack_size++] = value;
			} else
			if constexpr(
				same_as<jint,   ValueType> ||
				same_as<jshort, ValueType> ||
				same_as<jchar,  ValueType> ||
				same_as<jbyte,  ValueType>
			) {
				stack[stack_size++] = int32{ value.value };
			} else
			if constexpr(same_as<jfloat, ValueType>) {
				stack[stack_size++] = float{ value.value };
			}
			else {
				fputs("couldn't get", stderr); abort();
			}
		});
	};

	auto put = [&](field_value& value) {
		stack_entry stack_value = move(stack[--stack_size]);
		value.view([&]<typename ValueType>(ValueType& value) {
			if constexpr(same_as<reference, ValueType>) {
				value = move(stack_value.get<reference>());
			} else
			if constexpr(
				same_as<jint,   ValueType> ||
				same_as<jshort, ValueType> ||
				same_as<jchar,  ValueType> ||
				same_as<jbyte,  ValueType>
			) {
				value = ValueType {
					(decltype(value.value)) stack_value.get<int32>()
				};
			} else
			if constexpr(same_as<jbool, ValueType>) {
				value = jbool{ stack_value.get<int32>() == 1 };
			} else
			if constexpr(same_as<jfloat, ValueType>) {
				value = jfloat{ stack_value.get<float>() };
			}
			else {
				fputs("couldn't put", stderr); abort();
			}
		});
	};

	stack_entry result;

	reader([&]<typename Type>(Type x, uint8*& pc) {
		if constexpr (same_as<Type, nop>) {}
		else if constexpr (same_as<Type, a_const_null>) {
			if(info) { tabs(); fputs("a_const_null\n", stderr); }
			stack[stack_size++] = reference{};
		}
		else if constexpr (same_as<Type, i_const_0>) {
			if(info) { tabs(); fputs("i_const_0\n", stderr); }
			stack[stack_size++] = 0;
		}
		else if constexpr (same_as<Type, i_const_1>) {
			if(info) { tabs(); fputs("i_const_1\n", stderr); }
			stack[stack_size++] = 1;
		}
		else if constexpr (same_as<Type, i_const_2>) {
			if(info) { tabs(); fputs("i_const_2\n", stderr); }
			stack[stack_size++] = 2;
		}
		else if constexpr (same_as<Type, i_const_3>) {
			if(info) { tabs(); fputs("i_const_3\n", stderr); }
			stack[stack_size++] = 3;
		}
		else if constexpr (same_as<Type, i_const_4>) {
			if(info) { tabs(); fputs("i_const_4\n", stderr); }
			stack[stack_size++] = 4;
		}
		else if constexpr (same_as<Type, i_const_5>) {
			if(info) { tabs(); fputs("i_const_5\n", stderr); }
			stack[stack_size++] = 5;
		}
		else if constexpr (same_as<Type, bi_push>) {
			if(info) {
				tabs(); fputs("bi_push ", stderr);
				fprintf(stderr, "%hhd\n", x.value);
			}
			stack[stack_size++] = x.value;
		}
		else if constexpr (same_as<Type, ldc> || same_as<Type, ldc_w>) {
			if(info) {
				if constexpr(same_as<Type, ldc>) {
					tabs(); fputs("ldc ", stderr);
					fprintf(stderr, "%hhd\n", x.index);
				} else {
					tabs(); fputs("ldc_w ", stderr);
					fprintf(stderr, "%hd\n", x.index);
				}
			}
			const_pool_entry constatnt = m._class().constant(x.index);
			if(constatnt.is<class_file::constant::int32>()) {
				stack[stack_size++] = {
					constatnt.get<class_file::constant::int32>().value
				};
			} else
			if(constatnt.is<class_file::constant::float32>()) {
				stack[stack_size++] = {
					constatnt.get<class_file::constant::float32>().value
				};
			} else
			if(constatnt.is<class_file::constant::string>()) {
				stack[stack_size++] = c.get_string(x.index);
			}
			else {
				fputs("unknown constant", stderr); abort();
			}
		}
		else if constexpr (same_as<Type, i_load_0>) {
			if(info) { tabs(); fputs("i_load_0\n", stderr); }
			stack[stack_size++] = local[0].get<int32>();
		}
		else if constexpr (same_as<Type, i_load_1>) {
			if(info) { tabs(); fputs("i_load_1\n", stderr); }
			stack[stack_size++] = local[1].get<int32>();
		}
		else if constexpr (same_as<Type, i_load_2>) {
			if(info) { tabs(); fputs("i_load_2\n", stderr); }
			stack[stack_size++] = local[2].get<int32>();
		}
		else if constexpr (same_as<Type, i_load_3>) {
			if(info) { tabs(); fputs("i_load_3\n", stderr); }
			stack[stack_size++] = local[3].get<int32>();
		}
		else if constexpr (same_as<Type, a_load_0>) {
			if(info) { tabs(); fputs("a_load_0\n", stderr); }
			stack[stack_size++] = local[0].get<reference>();
		}
		else if constexpr (same_as<Type, a_load_1>) {
			if(info) { tabs(); fputs("a_load_1\n", stderr); }
			stack[stack_size++] = local[1].get<reference>();
		}
		else if constexpr (same_as<Type, i_a_load>) {
			if(info) { tabs(); fputs("i_a_load\n", stderr); }
			auto index = stack[--stack_size].get<int32>();
			auto ref = move(stack[--stack_size].get<reference>());
			int32* ptr = (int32*) ref.object().values()[0].get<jlong>().value;
			stack[stack_size++] = ptr[index];
		}
		else if constexpr (same_as<Type, i_store_0>) {
			if(info) { tabs(); fputs("i_store_0\n", stderr); }
			local[0] = stack[--stack_size].get<int32>();
		}
		else if constexpr (same_as<Type, i_store_1>) {
			if(info) { tabs(); fputs("i_store_1\n", stderr); }
			local[1] = stack[--stack_size].get<int32>();
		}
		else if constexpr (same_as<Type, i_store_2>) {
			if(info) { tabs(); fputs("i_store_2\n", stderr); }
			local[2] = stack[--stack_size].get<int32>();
		}
		else if constexpr (same_as<Type, i_store_3>) {
			if(info) { tabs(); fputs("i_store_3\n", stderr); }
			local[3] = stack[--stack_size].get<int32>();
		}
		else if constexpr (same_as<Type, a_store_0>) {
			if(info) { tabs(); fputs("a_store_0\n", stderr); }
			local[0] = move(stack[--stack_size].get<reference>());
		}
		else if constexpr (same_as<Type, a_store_1>) {
			if(info) { tabs(); fputs("a_store_1\n", stderr); }
			local[1] = move(stack[--stack_size].get<reference>());
		}
		else if constexpr (same_as<Type, i_a_store>) {
			if(info) { tabs(); fputs("i_a_store\n", stderr); }
			int32 value = stack[--stack_size].get<int32>();
			int32 index = stack[--stack_size].get<int32>();
			auto ref = move(stack[--stack_size].get<reference>());
			auto ptr = (int32*) ref.object().values()[0].get<jlong>().value;
			ptr[index] = value;
		}

		else if constexpr (same_as<Type, dup>) {
			if(info) { tabs(); fputs("dup\n", stderr); }
			auto& top = stack[stack_size - 1];
			stack[stack_size++] = top;
		}

		else if constexpr (same_as<Type, i_add>) {
			if(info) { tabs(); fputs("i_add\n", stderr); }
			int32 value2 = stack[--stack_size].get<int32>();
			int32 value1 = stack[--stack_size].get<int32>();
			stack[stack_size++] = value1 + value2;
		}
		else if constexpr (same_as<Type, i_sub>) {
			if(info) { tabs(); fputs("i_sub\n", stderr); }
			int32 value2 = stack[--stack_size].get<int32>();
			int32 value1 = stack[--stack_size].get<int32>();
			stack[stack_size++] = value1 - value2;
		}
		else if constexpr (same_as<Type, i_mul>) {
			if(info) { tabs(); fputs("i_mul\n", stderr); }
			int32 value2 = stack[--stack_size].get<int32>();
			int32 value1 = stack[--stack_size].get<int32>();
			stack[stack_size++] = value1 * value2;
		}
		else if constexpr (same_as<Type, i_div>) {
			if(info) { tabs(); fputs("i_div\n", stderr); }
			int32 value2 = stack[--stack_size].get<int32>();
			int32 value1 = stack[--stack_size].get<int32>();
			stack[stack_size++] = value1 / value2;
		}
		else if constexpr (same_as<Type, i_inc>) {
			if(info) { tabs(); fputs("i_inc\n", stderr); }
			local[x.index].template get<int32>() += x.value;
		}
		else if constexpr (same_as<Type, if_eq>) {
			if(info) {
				tabs(); fputs("if_eq ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack[--stack_size].get<int32>();
			if(value == 0) {
				pc += x.branch - sizeof(int16) - sizeof(uint8);
			}
		}
		else if constexpr (same_as<Type, if_ne>) {
			if(info) {
				tabs(); fputs("if_ne ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack[--stack_size].get<int32>();
			if(value != 0) {
				pc += x.branch - sizeof(int16) - sizeof(uint8);
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_ge>) {
			if(info) {
				tabs(); fputs("if_i_cmp_ge ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack[--stack_size].get<int32>();
			int32 value1 = stack[--stack_size].get<int32>();
			if(value1 >= value2) {
				pc += x.branch - sizeof(int16) - sizeof(uint8);
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_gt>) {
			if(info) {
				tabs(); fputs("if_i_cmp_gt ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack[--stack_size].get<int32>();
			int32 value1 = stack[--stack_size].get<int32>();
			if(value1 > value2) {
				pc += x.branch - sizeof(int16) - sizeof(uint8);
			}
		}
		else if constexpr (same_as<Type, go_to>) {
			if(info) {
				tabs(); fputs("go_to ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			pc += x.branch - sizeof(int16) - sizeof(uint8);
		}
		else if constexpr (same_as<Type, i_ret>) {
			if(info) { tabs(); fputs("i_ret\n", stderr); }
			result = stack[--stack_size].get<int32>();
			return true;
		}
		else if constexpr (same_as<Type, a_ret>) {
			if(info) { tabs(); fputs("a_ret\n", stderr); }
			result = stack[--stack_size].get<reference>();
			return true;
		}
		else if constexpr (same_as<Type, ret>) {
			if(info) { tabs(); fputs("ret\n", stderr); }
			return true;
		}
		else if constexpr (same_as<Type, get_static>) {
			if(info) {
				tabs(); fputs("get_static ", stderr);
				fprintf(stderr, "%hd\n", x.index);
			}
			field& f = c.get_static_field(x.index);
			auto& value = ((static_field*)&f)->value();
			get(value);
		}
		else if constexpr (same_as<Type, put_static>) {
			if(info) {
				tabs(); fputs("put_static ", stderr);
				fprintf(stderr, "%hd\n", x.index);
			}
			field& field = c.get_static_field(x.index);
			field_value& static_field_value = ((static_field*)&field)->value();
			put(static_field_value);
		}
		else if constexpr (same_as<Type, get_field>) {
			if(info) {
				tabs(); fputs("get_field ", stderr);
				auto field_ref = c.field_ref_constant(x.index);
				cc::name_and_type nat = c.name_and_type_constant(
					field_ref.name_and_type_index
				);
				auto class_ = c.class_constant(field_ref.class_index);
				auto class_name = c.utf8_constant(class_.name_index);
				auto name = c.utf8_constant(nat.name_index);
				fwrite(class_name.data(), 1, class_name.size(), stderr);
				fputc('.', stderr);
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			auto instance_field_index {
				c.get_resolved_instance_field_index(x.index)
			};

			reference ref = move(stack[--stack_size].get<reference>());
			field_value& value = ref.object()[instance_field_index];
			get(value);
		}
		else if constexpr (same_as<Type, put_field>) {
			if(info) {
				tabs(); fputs("put_field ", stderr);
				auto field_ref = c.field_ref_constant(x.index);
				cc::name_and_type nat = c.name_and_type_constant(
					field_ref.name_and_type_index
				);
				auto class_ = c.class_constant(field_ref.class_index);
				auto class_name = c.utf8_constant(class_.name_index);
				auto name = c.utf8_constant(nat.name_index);
				fwrite(class_name.data(), 1, class_name.size(), stderr);
				fputc('.', stderr);
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			auto instance_field_index {
				c.get_resolved_instance_field_index(x.index)
			};

			stack_entry stack_value = move(stack[--stack_size]);
			reference ref = move(stack[--stack_size].get<reference>());
			field_value& field_value = ref.object()[instance_field_index];
			put(field_value);
		}
		else if constexpr (same_as<Type, invoke_virtual>) {
			cc::method_ref method_ref_info {
				c.method_ref_constant(x.index)
			};
			cc::name_and_type name_and_type_info {
				c.name_and_type_constant(method_ref_info.name_and_type_index)
			};

			auto name = c.utf8_constant(name_and_type_info.name_index);
			auto desc = c.utf8_constant(name_and_type_info.descriptor_index);

			if(info) {
				cc::_class class_info {
					c.class_constant(method_ref_info.class_index)
				};
				auto class_name = c.utf8_constant(class_info.name_index);
				tabs(); fputs("invoke_virtual ", stderr);
				fwrite(class_name.data(), 1, class_name.size(), stderr);
				fputc('.', stderr);
				fwrite(name.data(), 1, name.size(), stderr);
				fwrite(desc.data(), 1, desc.size(), stderr);
				fputc('\n', stderr);
			}

			class_file::descriptor::method_reader params_reader{ desc.begin() };
			uint16 args_count = 0;
			params_reader([&](auto){ ++args_count; return true; });

			method* m0 = nullptr;
			_class* c0 = &stack[stack_size - args_count - 1]
				.get<reference>()
				.object()
				._class();

			while(true) {
				if(m0 = c0->try_find_method(name, desc); m0 != nullptr) {
					break;
				}
				if(c0->super_class_index() == 0) {
					break;
				}
				c0 = &c0->get_class(c0->super_class_index());
			}

			if(m0 == nullptr) {
				nuint index = 0;
				c.for_each_maximally_specific_superinterface_method(
					name, desc,
					[&](method& m) {
						if(index++ == 0) {
							m0 = &m;
							return;
						}
						fputs(
							"more than one maximally-specific interface method",
							stderr
						);
						abort();
					}
				);
			}

			if(m0 == nullptr) {
				fputs("couldn't find method", stderr); abort();
			}

			++args_count; // this
			stack_size -= args_count;
			stack_entry result = execute(
				*m0, span{ stack + stack_size, args_count }
			);
			if(!result.is<jvoid>()) {
				stack[stack_size++] = result;
			}
		}
		else if constexpr (same_as<Type, invoke_special>) {
			method& m0 = c.get_resolved_method(x.index);
			auto desc = m0.descriptor();

			if(info) {
				tabs(); fputs("invoke_special ", stderr);
				auto name = m0._class().name();
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('.', stderr);
				fwrite(m0.name().data(), 1, m0.name().size(), stderr);
				fputc('\n', stderr);
			}

			class_file::descriptor::method_reader params_reader{ desc.begin() };
			uint16 args_count = 0;
			params_reader([&](auto){ ++args_count; return true; });
			++args_count; // this
			stack_size -= args_count;
			stack_entry result = execute(
				m0, span{ stack + stack_size, args_count }
			);
			if(!result.is<jvoid>()) {
				stack[stack_size++] = result;
			}
		}
		else if constexpr (same_as<Type, invoke_static>) {
			method& next_method = c.get_method(x.index);
			auto desc = next_method.descriptor();

			class_file::descriptor::method_reader params_reader{ desc.begin() };
			uint16 args_count = 0;
			params_reader([&](auto){ ++args_count; return true; });
			stack_size -= args_count;
			stack_entry result = execute(
				next_method,
				span{ stack + stack_size, args_count }
			);
			if(!result.is<jvoid>()) {
				stack[stack_size++] = result;
			}
		}
		else if constexpr (same_as<Type, invoke_interface>) {
			cc::method_ref method_ref_info {
				c.method_ref_constant(x.index)
			};
			cc::name_and_type name_and_type_info {
				c.name_and_type_constant(method_ref_info.name_and_type_index)
			};

			auto name = c.utf8_constant(name_and_type_info.name_index);
			auto desc = c.utf8_constant(name_and_type_info.descriptor_index);

			if(info) {
				cc::_class class_info {
					c.class_constant(method_ref_info.class_index)
				};
				auto class_name = c.utf8_constant(class_info.name_index);
				tabs(); fputs("invoke_interface ", stderr);
				fwrite(class_name.data(), 1, class_name.size(), stderr);
				fputc('.', stderr);
				fwrite(name.data(), 1, name.size(), stderr);
				fwrite(desc.data(), 1, desc.size(), stderr);
				fputc('\n', stderr);
			}

			uint16 args_count = x.count;

			method* m0 = nullptr;
			_class* c0 = &stack[stack_size - args_count]
				.get<reference>()
				.object()
				._class();

			while(true) {
				if(m0 = c0->try_find_method(name, desc); m0 != nullptr) {
					break;
				}
				if(c0->super_class_index() == 0) {
					break;
				}
				c0 = &c0->get_class(c0->super_class_index());
			}

			if(m0 == nullptr) {
				nuint index = 0;
				c.for_each_maximally_specific_superinterface_method(
					name, desc,
					[&](method& m) {
						if(index++ == 0) {
							m0 = &m;
							return;
						}
						fputs(
							"more than one maximally-specific interface method",
							stderr
						);
						abort();
					}
				);
			}

			if(m0 == nullptr) {
				fputs("couldn't find method", stderr); abort();
			}

			stack_size -= args_count;
			stack_entry result = execute(
				*m0, span{ stack + stack_size, args_count }
			);
			if(!result.is<jvoid>()) {
				stack[stack_size++] = result;
			}
		}
		else if constexpr (same_as<Type, _new>) {
			if(info) {
				tabs(); fputs("new ", stderr);
				auto name = c.utf8_constant(
					c.class_constant(x.index).name_index
				);
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}
			_class& c0 = c.get_class(x.index);
			stack[stack_size++] = objects.find_free(c0);
		}
		else if constexpr (same_as<Type, new_array>) {
			c_string<c_string_type::known_size> name;
			nuint size = 0;

			switch (x.type) {
				case 4:  name = "boolean"; size = sizeof(jbool);   break;
				case 5:  name = "char";    size = sizeof(jchar);   break;
				case 6:  name = "float";   size = sizeof(jfloat);  break;
				case 7:  name = "double";  size = sizeof(jdouble); break;
				case 8:  name = "byte";    size = sizeof(jbyte);   break;
				case 9:  name = "short";   size = sizeof(jshort);  break;
				case 10: name = "int";     size = sizeof(jint);    break;
				case 11: name = "long";    size = sizeof(jlong);   break;
				default:
					fputs("unknown type of array", stderr);
					abort();
			}

			if(info) {
				tabs(); fputs("new_array ", stderr);
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			int32 count = stack[--stack_size].get<int32>();
			_class& c0 = find_or_load(
				concat_view{ name, array{'[', ']'} }
			);

			auto ref = objects.find_free(c0);
			ref.object().values()[0] = field_value {
				jlong {
					(int64) default_allocator{}.allocate_zeroed(
						count * size
					)
				}
			};
			ref.object().values()[1] = jint{ count };
			stack[stack_size++] = move(ref);
		}
		else if constexpr (same_as<Type, a_new_array>) {
			_class& element_class = c.get_class(x.index);
			auto name = element_class.name();

			if(info) {
				tabs(); fputs("a_new_array ", stderr);
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			int32 count = stack[--stack_size].get<int32>();
			_class& c0 = find_or_load(
				concat_view{ name, array{'[', ']'} }
			);

			auto ref = objects.find_free(c0);
			ref.object().values()[0] = field_value {
				jlong {
					(int64) default_allocator{}.allocate_zeroed(
						count * sizeof(reference)
					)
				}
			};
			ref.object().values()[1] = jint { count };
			stack[stack_size++] = move(ref);
		}
		else if constexpr (same_as<Type, array_length>) {
			if(info) { tabs(); fputs("array_length\n", stderr); }
			auto ref = stack[--stack_size].get<reference>();
			stack[stack_size++] = ref.object().values()[0].get<jint>().value;
		}
		else if constexpr (same_as<Type, uint8>) {
			fprintf(stderr, "unknown instruction ");
			for_each_digit_in_number(
				number{ x }, base{ 10 },
				[](auto digit) { fputc('0' + digit, stderr); }
			);
			abort();
		}
		else {
			fprintf(stderr, "unimplemented instruction ");
			for_each_digit_in_number(
				number{ Type::code }, base{ 10 },
				[](auto digit) { fputc('0' + digit, stderr); }
			);
			abort();
		}

	}, m.code().size());

	if(info) {
		--tab;
	}

	return result;
}