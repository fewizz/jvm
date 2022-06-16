#pragma once

#include "execute/definition.hpp"
#include "execute/info.hpp"
#include "execute/get_field_value.hpp"
#include "execute/put_field_value.hpp"
#include "execute/ldc.hpp"
#include "execute/invoke_virtual.hpp"
#include "execute/invoke_special.hpp"
#include "execute/invoke_static.hpp"
#include "execute/invoke_interface.hpp"
#include "execute/new_array.hpp"
#include "class/file/reader.hpp"
#include "class/file/descriptor/reader.hpp"
#include "object/create.hpp"
#include "native.hpp"
#include "../abort.hpp"

#include <stdio.h>
#include <core/number.hpp>
#include <core/c_string.hpp>
#include <core/concat.hpp>
#include <core/single.hpp>

inline stack_entry execute(method& m, span<stack_entry, uint16> args) {
	namespace cf = class_file;
	namespace instr = cf::code::instruction;

	_class& c = m._class();

	if(info) {
		++tab;
		tabs();
		fwrite(c.name().data(), 1, c.name().size(), stderr);
		fputc('.', stderr);
		fwrite(m.name().data(), 1, m.name().size(), stderr);
		fputs("()\n", stderr);
	}

	if(m.is_native()) {
		if(m.function_ptr() == nullptr) {
			concat_view full_name {
				m._class().name(),
				single_view{ '/' },
				m.name()
			};
			m.function_ptr() = try_find_native_method(full_name);
		}

		//auto ptr = m.function_ptr();

		// TODO
		//if(args.size() == 0) {
		//}
	}

	if(m.code().data() == nullptr) {
		fputs("no code", stderr); abort();
	}

	cf::attribute::code::reader<
		uint8*,
		cf::attribute::code::reader_stage::code
	> reader{ m.code().data() };

	stack_entry stack[m.code().max_stack];
	nuint stack_size = 0;

	stack_entry local[m.code().max_locals];
	for(int i = 0; i < args.size(); ++i) local[i] = move(args[i]);

	using namespace cf::code::instruction;
	namespace cc = cf::constant;

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
		else if constexpr (same_as<Type, instr::ldc>) {
			::ldc(c, x, stack, stack_size);
		}
		else if constexpr (same_as<Type, instr::ldc_w>) {
			::ldc_w(c, x, stack, stack_size);
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
		else if constexpr (same_as<Type, a_load_2>) {
			if(info) { tabs(); fputs("a_load_2\n", stderr); }
			stack[stack_size++] = local[2].get<reference>();
		}
		else if constexpr (same_as<Type, a_load_3>) {
			if(info) { tabs(); fputs("a_load_3\n", stderr); }
			stack[stack_size++] = local[3].get<reference>();
		}
		else if constexpr (same_as<Type, i_a_load>) {
			if(info) { tabs(); fputs("i_a_load\n", stderr); }
			auto index = stack[--stack_size].get<int32>();
			auto ref = move(stack[--stack_size].get<reference>());
			int32* ptr = (int32*) ref.object().values()[0].get<jlong>().value;
			stack[stack_size++] = ptr[index];
		}
		else if constexpr (same_as<Type, b_a_load>) {
			if(info) { tabs(); fputs("b_a_load\n", stderr); }
			auto index = stack[--stack_size].get<int32>();
			auto ref = move(stack[--stack_size].get<reference>());
			int8* ptr = (int8*) ref.object().values()[0].get<jlong>().value;
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
		else if constexpr (same_as<Type, a_store_2>) {
			if(info) { tabs(); fputs("a_store_2\n", stderr); }
			local[2] = move(stack[--stack_size].get<reference>());
		}
		else if constexpr (same_as<Type, a_store_3>) {
			if(info) { tabs(); fputs("a_store_3\n", stderr); }
			local[3] = move(stack[--stack_size].get<reference>());
		}
		else if constexpr (same_as<Type, i_a_store>) {
			if(info) { tabs(); fputs("i_a_store\n", stderr); }
			int32 value = stack[--stack_size].get<int32>();
			int32 index = stack[--stack_size].get<int32>();
			auto ref = move(stack[--stack_size].get<reference>());
			auto ptr = (int32*) ref.object().values()[0].get<jlong>().value;
			ptr[index] = value;
		}
		else if constexpr (same_as<Type, b_a_store>) {
			if(info) { tabs(); fputs("b_a_store\n", stderr); }
			int32 value = stack[--stack_size].get<int32>();
			int32 index = stack[--stack_size].get<int32>();
			auto ref = move(stack[--stack_size].get<reference>());
			auto ptr = (int8*) ref.object().values()[0].get<jlong>().value;
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
		else if constexpr (same_as<Type, i_to_b>) {
			if(info) { tabs(); fputs("i_to_b\n", stderr); }
			stack[stack_size - 1] = (int8) stack[stack_size - 1].get<int32>();
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
			get_field_value(value, stack, stack_size);
		}
		else if constexpr (same_as<Type, put_static>) {
			if(info) {
				tabs(); fputs("put_static ", stderr);
				fprintf(stderr, "%hd\n", x.index);
			}
			field& field = c.get_static_field(x.index);
			field_value& static_field_value = ((static_field*)&field)->value();
			put_field_value(static_field_value, stack, stack_size);
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
			get_field_value(value, stack, stack_size);
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
			put_field_value(field_value, stack, stack_size);
		}
		else if constexpr (same_as<Type, instr::invoke_virtual>) {
			::invoke_virtual(c, x, stack, stack_size);
		}
		else if constexpr (same_as<Type, instr::invoke_special>) {
			::invoke_special(c, x, stack, stack_size);
		}
		else if constexpr (same_as<Type, instr::invoke_static>) {
			::invoke_static(c, x, stack, stack_size);
		}
		else if constexpr (same_as<Type, instr::invoke_interface>) {
			::invoke_interface(c, x, stack, stack_size);
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
			stack[stack_size++] = create_object(c0);
		}
		else if constexpr (same_as<Type, instr::new_array>) {
			::new_array(/* c, */ x, stack, stack_size);
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
			_class& c0 = find_or_load_class(
				concat_view{ name, array{'[', ']'} }
			);

			auto ref = create_object(c0);
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