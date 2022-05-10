#pragma once

#include "class.hpp"

inline uint32 execute(const method& m, span<uint32, uint16> args = {});

#include "class/file/reader.hpp"
#include "class/file/descriptor/reader.hpp"
#include "../abort.hpp"

#include <stdio.h>

uint32 execute(const method& m, span<uint32, uint16> args) {
	class_file::attribute::code::reader<
		uint8*,
		class_file::attribute::code::reader_stage::code
	> reader{ m.code().data() };

	uint32 stack[m.code().max_stack];
	nuint stack_size = 0;

	uint32 local[m.code().max_locals];
	for(int i = 0; i < m.code().max_locals; ++i) local[0] = 0;
	for(int i = 0; i < args.size(); ++i) local[i] = args[i];

	using namespace class_file;
	using namespace code::instruction;

	uint32 result = 0;

	reader([&]<typename Type>(Type x, uint8*& pc) {
		if constexpr (same_as<Type, nop>) {}
		else if constexpr (same_as<Type, i_const_0>) {
			stack[stack_size++] = 0;
		}
		else if constexpr (same_as<Type, i_const_1>) {
			stack[stack_size++] = 1;
		}
		else if constexpr (same_as<Type, i_const_2>) {
			stack[stack_size++] = 2;
		}
		else if constexpr (same_as<Type, i_const_3>) {
			stack[stack_size++] = 3;
		}
		else if constexpr (same_as<Type, i_const_4>) {
			stack[stack_size++] = 4;
		}
		else if constexpr (same_as<Type, i_const_5>) {
			stack[stack_size++] = 5;
		}
		else if constexpr (same_as<Type, bi_push>) {
			stack[stack_size++] = int32(x.value);
		}
		else if constexpr (same_as<Type, ldc>) {
			auto v = m._class().int32_constant(x.index);
			stack[stack_size++] = v.value; // even if it is float
		}
		else if constexpr (same_as<Type, i_load_0>) {
			stack[stack_size++] = local[0];
		}
		else if constexpr (same_as<Type, i_load_1>) {
			stack[stack_size++] = local[1];
		}
		else if constexpr (same_as<Type, i_store_0>) {
			local[0] = stack[--stack_size];
		}
		else if constexpr (same_as<Type, i_store_1>) {
			local[1] = stack[--stack_size];
		}
		else if constexpr (same_as<Type, i_ret>) {
			result = stack[--stack_size];
			return true;
		}
		else if constexpr (same_as<Type, ret>) {
			return true;
		}
		else if constexpr (same_as<Type, get_static>) {
			//auto field_ref = m._class().field_ref_constant(x.index);
			//auto class_info = m._class().class_constant(field_ref.class_index);
			//auto class_name = m._class().utf8_constant(class_info.name_index);
			//auto nat = m._class().name_and_type_constant(
			//	field_ref.name_and_type_index
			//);
			::field& f = m._class().get_field(x.index);
			auto& val = ((static_field*)&f)->value();
			if(val.is<int32>()) {
				stack[stack_size++] = val.get<int32>();
			}
			else {
				fputs("unknown static", stderr);
				abort();
			}
		}
		else if constexpr (same_as<Type, put_static>) {
			//auto field_ref = m._class().field_ref_constant(x.index);
			//auto class_info = m._class().class_constant(field_ref.class_index);
			//auto class_name = m._class().utf8_constant(class_info.name_index);
			//auto nat = m._class().name_and_type_constant(
			//	field_ref.name_and_type_index
			//);
			
			::field& f = m._class().get_field(x.index);
			auto& val = ((static_field*)&f)->value();
			if(val.is<int32>()) {
				val.get<int32>() = stack[--stack_size];
			}
			else {
				fputs("unknown static", stderr);
				abort();
			}
		}
		else if constexpr (same_as<Type, i_add>) {
			int32 value2 = stack[--stack_size];
			int32 value1 = stack[--stack_size];
			stack[stack_size++] = value1 + value2;
		}
		else if constexpr (same_as<Type, i_sub>) {
			int32 value2 = stack[--stack_size];
			int32 value1 = stack[--stack_size];
			stack[stack_size++] = value1 - value2;
		}
		else if constexpr (same_as<Type, i_mul>) {
			int32 value2 = stack[--stack_size];
			int32 value1 = stack[--stack_size];
			stack[stack_size++] = value1 * value2;
		}
		else if constexpr (same_as<Type, i_div>) {
			int32 value2 = stack[--stack_size];
			int32 value1 = stack[--stack_size];
			stack[stack_size++] = value1 / value2;
		}
		else if constexpr (same_as<Type, i_inc>) {
			local[x.index] += int32(x.value);
		}
		else if constexpr (same_as<Type, if_i_cmp_ge>) {
			int32 value2 = int32(stack[--stack_size]);
			int32 value1 = int32(stack[--stack_size]);
			if(value1 >= value2) {
				pc += x.branch - sizeof(int16) - sizeof(uint8);
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_gt>) {
			int32 value2 = int32(stack[--stack_size]);
			int32 value1 = int32(stack[--stack_size]);
			if(value1 > value2) {
				pc += x.branch - sizeof(int16) - sizeof(uint8);
			}
		}
		else if constexpr (same_as<Type, go_to>) {
			pc += x.branch - sizeof(int16) - sizeof(uint8);
		}
		else if constexpr (same_as<Type, invoke_static>) {
			::method& next_method = m._class().get_method(x.index);
			auto desc = next_method.descriptor();

			using namespace descriptor;
			method_reader params_reader{ desc.begin() };
			uint16 args_count = 0;
			params_reader([&](auto){ ++args_count; return true; });
			stack_size -= args_count;
			stack[stack_size] = execute(
				next_method,
				span{ stack + stack_size, args_count }
			);
			++stack_size;
		}
		else {
			fprintf(stderr, "unimplemented instruction");
			abort();
		}

	}, m.code().size());

	return result;
}