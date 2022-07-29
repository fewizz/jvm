#pragma once

#include "./get_field_value.hpp"
#include "./put_field_value.hpp"
#include "./ldc.hpp"
#include "./invoke_virtual.hpp"
#include "./invoke_special.hpp"
#include "./invoke_static.hpp"
#include "./invoke_interface.hpp"
#include "./new_array.hpp"

#include "execute.hpp"
#include "execution/info.hpp"
#include "thrown.hpp"
#include "execution/latest_context.hpp"
#include "array.hpp"
#include "object/create.hpp"
#include "native/function/s/find.hpp"
#include "abort.hpp"
#include "lib/java/lang/null_pointer_exception.hpp"
#include "lib/java/lang/index_out_of_bounds_exception.hpp"

#include <class_file/reader.hpp>
#include <class_file/descriptor/reader.hpp>

#include <core/number.hpp>
#include <core/c_string.hpp>
#include <core/concat.hpp>
#include <core/single.hpp>
#include <core/on_scope_exit.hpp>
#include <core/max.hpp>

#include <stdio.h>
#include <math.h>

static stack_entry execute(
	method_with_class mwc,
	arguments_container args
) {
	namespace cf = class_file;
	namespace instr = cf::attribute::code::instruction;

	_class& c = mwc._class();
	method& m = mwc.method();

	if(info) {
		tabs();
		fputs("executing: ", stderr);
		fwrite(c.name().data(), 1, c.name().size(), stderr);
		fputc('.', stderr);
		fwrite(c.name(m).data(), 1, c.name(m).size(), stderr);
		fwrite(c.descriptor(m).data(), 1, c.descriptor(m).size(), stderr);
		fputc(' ', stderr);
		fprintf(stderr, "max_stack: %hu", m.code().max_stack);
		fputc('\n', stderr);
		++tab;
	}

	on_scope_exit bring_tab_back {
		[] { if(info) { --tab; } }
	};

	stack_entry result;
	uint32 pc = 0;

	execution_context ctx {
		c, m, latest_execution_context
	};

	latest_execution_context = ctx;

	on_scope_exit set_latest_execution_context_to_previous {
		[&] { latest_execution_context = ctx.previous; }
	};

	if(m.is_native()) {
		if(!m.has_native_function()) {
			m.native_function(find_native_function(mwc));
		}
		auto& native_function = m.native_function();
		return native_function.call(args);
	}

	if(m.code().data() == nullptr) {
		fputs("no code", stderr); abort();
	}

	cf::attribute::code::reader<
		uint8*,
		cf::attribute::code::reader_stage::code
	> reader{ m.code().data() };

	stack_entry stack[max(m.code().max_stack, 1)]; // ub if 0
	nuint stack_size = 0;

	stack_entry local[m.code().max_locals * 2]; // there may be longs or doubles

	{
		uint16 local_index = 0;
		uint16 arg_index = 0;
		for(;arg_index < args.size(); ++arg_index, ++local_index) {
			local[local_index] = move(args[arg_index]);
			if(
				local[local_index].is<jlong>() ||
				local[local_index].is<jdouble>()
			) {
				++local_index;
			}
		}
	}

	namespace attr = cf::attribute;
	using namespace attr::code::instruction;
	namespace cc = cf::constant;

	auto on_unimplemented_instruction = [] (uint8 code) {
		if(info) tabs();
		fprintf(stderr, "unimplemented instruction ");
		for_each_digit_in_number(
			number{ code }, base{ 10 },
			[](auto digit) { fputc('0' + digit, stderr); }
		);
		abort();
	};

	reader([&]<typename Type>(Type x, uint8*& it) {
		on_scope_exit update_pc{[&](){
			pc = it - m.code().begin();
		}};

		auto handle_thrown = [&]() -> loop_action {
			if(thrown.is_null()) {
				return loop_action::next;
			}

			_class& thrown_class = thrown.object()._class();

			auto& exception_handlers = m.exception_handlers();

			for(attr::code::exception_handler handler : exception_handlers) {
				bool in_range = pc >= handler.start_pc && pc < handler.end_pc;
				if(!in_range) {
					continue;
				}

				_class& catch_class = c.get_class(handler.catch_type);

				bool same = &thrown_class == &catch_class;
				bool subclass = thrown_class.is_subclass_of(catch_class);

				if(!(same || subclass)) {
					continue;
				}

				it = m.code().begin() + handler.handler_pc;
				stack_size = 0;
				stack[stack_size++] = move(thrown);
				return loop_action::next;
			}

			return loop_action::stop;
		};

		auto view_array = [&]<typename E, typename Handler>(
			Handler&& handler
		) {
			int32 element_index = stack[--stack_size].get<jint>();
			reference array_ref = move(stack[--stack_size].get<reference>());
			if(array_ref.is_null()) {
				thrown = create_null_pointer_exception();
				return handle_thrown();
			}
			int32 len = ::array_length(array_ref.object());
			if(element_index < 0 || element_index >= len) {
				thrown = create_index_of_of_bounds_exception();
				return handle_thrown();
			}
			E* ptr = array_data<E>(array_ref.object());
			handler(ptr[element_index]);
			return loop_action::next;
		};

		if constexpr (same_as<Type, nop>) {}
		else if constexpr (same_as<Type, a_const_null>) {
			if(info) { tabs(); fputs("a_const_null\n", stderr); }
			stack[stack_size++] = reference{};
		}
		else if constexpr (same_as<Type, i_const_m1>) {
			if(info) { tabs(); fputs("i_const_m1\n", stderr); }
			stack[stack_size++] = jint{ -1 };
		}
		else if constexpr (same_as<Type, i_const_0>) {
			if(info) { tabs(); fputs("i_const_0\n", stderr); }
			stack[stack_size++] = jint{ 0 };
		}
		else if constexpr (same_as<Type, i_const_1>) {
			if(info) { tabs(); fputs("i_const_1\n", stderr); }
			stack[stack_size++] = jint{ 1 };
		}
		else if constexpr (same_as<Type, i_const_2>) {
			if(info) { tabs(); fputs("i_const_2\n", stderr); }
			stack[stack_size++] = jint{ 2 };
		}
		else if constexpr (same_as<Type, i_const_3>) {
			if(info) { tabs(); fputs("i_const_3\n", stderr); }
			stack[stack_size++] = jint{ 3 };
		}
		else if constexpr (same_as<Type, i_const_4>) {
			if(info) { tabs(); fputs("i_const_4\n", stderr); }
			stack[stack_size++] = jint{ 4 };
		}
		else if constexpr (same_as<Type, i_const_5>) {
			if(info) { tabs(); fputs("i_const_5\n", stderr); }
			stack[stack_size++] = jint{ 5 };
		}
		else if constexpr (same_as<Type, l_const_0>) {
			if(info) { tabs(); fputs("l_const_0\n", stderr); }
			stack[stack_size++] = jlong{ 0 };
		}
		else if constexpr (same_as<Type, l_const_1>) {
			if(info) { tabs(); fputs("l_const_1\n", stderr); }
			stack[stack_size++] = jlong{ 1 };
		}
		else if constexpr (same_as<Type, f_const_0>) {
			if(info) { tabs(); fputs("f_const_0\n", stderr); }
			stack[stack_size++] = jfloat{ 0.0F };
		}
		else if constexpr (same_as<Type, f_const_1>) {
			if(info) { tabs(); fputs("f_const_1\n", stderr); }
			stack[stack_size++] = jfloat{ 1.0F };
		}
		else if constexpr (same_as<Type, f_const_2>) {
			if(info) { tabs(); fputs("f_const_2\n", stderr); }
			stack[stack_size++] = jfloat{ 2.0F };
		}
		else if constexpr (same_as<Type, bi_push>) {
			if(info) {
				tabs(); fputs("bi_push ", stderr);
				fprintf(stderr, "%hhd\n", x.value);
			}
			stack[stack_size++] = jint{ x.value };
		}
		else if constexpr (same_as<Type, si_push>) {
			if(info) {
				tabs(); fputs("si_push ", stderr);
				fprintf(stderr, "%hd\n", x.value);
			}
			stack[stack_size++] = jint{ x.value };
		}
		else if constexpr (same_as<Type, instr::ldc>) {
			::ldc(constant_index{ x.index }, c, stack, stack_size);
		}
		else if constexpr (same_as<Type, instr::ldc_w>) {
			::ldc_w(wide_constant_index{ x.index }, c, stack, stack_size);
		}
		else if constexpr (same_as<Type, instr::ldc_2_w>) {
			::ldc_2_w(wide_constant_index{ x.index }, c, stack, stack_size);
		}

		else if constexpr (same_as<Type, i_load>) {
			if(info) {
				tabs(); fputs("i_load ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			stack[stack_size++] = local[x.index].template get<jint>();
		}
		else if constexpr (same_as<Type, l_load>) {
			if(info) {
				tabs(); fputs("l_load ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			stack[stack_size++] = local[x.index].template get<jlong>();
		}
		else if constexpr (same_as<Type, f_load>) {
			if(info) {
				tabs(); fputs("f_load ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			stack[stack_size++] = local[x.index].template get<jfloat>();
		}
		else if constexpr (same_as<Type, a_load>) {
			if(info) {
				tabs(); fputs("a_load ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			stack[stack_size++] = local[x.index].template get<reference>();
		}
		else if constexpr (same_as<Type, i_load_0>) {
			if(info) { tabs(); fputs("i_load_0\n", stderr); }
			stack[stack_size++] = local[0].get<jint>();
		}
		else if constexpr (same_as<Type, i_load_1>) {
			if(info) { tabs(); fputs("i_load_1\n", stderr); }
			stack[stack_size++] = local[1].get<jint>();
		}
		else if constexpr (same_as<Type, i_load_2>) {
			if(info) { tabs(); fputs("i_load_2\n", stderr); }
			stack[stack_size++] = local[2].get<jint>();
		}
		else if constexpr (same_as<Type, i_load_3>) {
			if(info) { tabs(); fputs("i_load_3\n", stderr); }
			stack[stack_size++] = local[3].get<jint>();
		}
		else if constexpr (same_as<Type, l_load_0>) {
			if(info) { tabs(); fputs("l_load_0\n", stderr); }
			stack[stack_size++] = local[0].get<jlong>();
		}
		else if constexpr (same_as<Type, l_load_1>) {
			if(info) { tabs(); fputs("l_load_1\n", stderr); }
			stack[stack_size++] = local[1].get<jlong>();
		}
		else if constexpr (same_as<Type, l_load_2>) {
			if(info) { tabs(); fputs("l_load_2\n", stderr); }
			stack[stack_size++] = local[2].get<jlong>();
		}
		else if constexpr (same_as<Type, l_load_3>) {
			if(info) { tabs(); fputs("l_load_3\n", stderr); }
			stack[stack_size++] = local[3].get<jlong>();
		}
		else if constexpr (same_as<Type, a_load_0>) {
			if(info) { tabs(); fputs("a_load_0\n", stderr); }
			reference ref = local[0].get<reference>();
			stack[stack_size++] = move(ref);
		}
		else if constexpr (same_as<Type, a_load_1>) {
			if(info) { tabs(); fputs("a_load_1\n", stderr); }
			reference ref = local[1].get<reference>();
			stack[stack_size++] = move(ref);
		}
		else if constexpr (same_as<Type, a_load_2>) {
			if(info) { tabs(); fputs("a_load_2\n", stderr); }
			reference ref = local[2].get<reference>();
			stack[stack_size++] = move(ref);
		}
		else if constexpr (same_as<Type, a_load_3>) {
			if(info) { tabs(); fputs("a_load_3\n", stderr); }
			reference ref = local[3].get<reference>();
			stack[stack_size++] = move(ref);
		}
		else if constexpr (same_as<Type, i_a_load>) {
			if(info) { tabs(); fputs("i_a_load\n", stderr); }
			return view_array.template operator()<int32>([&](int32& v) {
				stack[stack_size++] = jint{ v };
			});
		}
		else if constexpr (same_as<Type, a_a_load>) {
			if(info) { tabs(); fputs("a_a_load\n", stderr); }
			return view_array.template operator()<reference>([&](reference& v) {
				stack[stack_size++] = v;
			});
		}
		else if constexpr (same_as<Type, b_a_load>) {
			if(info) { tabs(); fputs("b_a_load\n", stderr); }
			return view_array.template operator()<int8>([&](int8& v) {
				stack[stack_size++] = jint{ v };
			});
		}
		else if constexpr (same_as<Type, i_store>) {
			if(info) {
				tabs(); fputs("i_store ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			local[x.index] = stack[--stack_size].get<jint>();
		}
		else if constexpr (same_as<Type, l_store>) {
			if(info) {
				tabs(); fputs("l_store ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			local[x.index] = stack[--stack_size].get<jlong>();
		}
		else if constexpr (same_as<Type, f_store>) {
			if(info) {
				tabs(); fputs("f_store ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			local[x.index] = stack[--stack_size].get<jfloat>();
		}
		else if constexpr (same_as<Type, a_store>) {
			if(info) {
				tabs(); fputs("a_store ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			local[x.index] = move(stack[--stack_size].get<reference>());
		}
		else if constexpr (same_as<Type, i_store_0>) {
			if(info) { tabs(); fputs("i_store_0\n", stderr); }
			local[0] = stack[--stack_size].get<jint>();
		}
		else if constexpr (same_as<Type, i_store_1>) {
			if(info) { tabs(); fputs("i_store_1\n", stderr); }
			local[1] = stack[--stack_size].get<jint>();
		}
		else if constexpr (same_as<Type, i_store_2>) {
			if(info) { tabs(); fputs("i_store_2\n", stderr); }
			local[2] = stack[--stack_size].get<jint>();
		}
		else if constexpr (same_as<Type, i_store_3>) {
			if(info) { tabs(); fputs("i_store_3\n", stderr); }
			local[3] = stack[--stack_size].get<jint>();
		}
		else if constexpr (same_as<Type, l_store_0>) {
			if(info) { tabs(); fputs("l_store_0\n", stderr); }
			local[0] = stack[--stack_size].get<jlong>();
		}
		else if constexpr (same_as<Type, l_store_1>) {
			if(info) { tabs(); fputs("l_store_1\n", stderr); }
			local[1] = stack[--stack_size].get<jlong>();
		}
		else if constexpr (same_as<Type, l_store_2>) {
			if(info) { tabs(); fputs("l_store_2\n", stderr); }
			local[2] = stack[--stack_size].get<jlong>();
		}
		else if constexpr (same_as<Type, l_store_3>) {
			if(info) { tabs(); fputs("l_store_3\n", stderr); }
			local[3] = stack[--stack_size].get<jlong>();
		}
		else if constexpr (same_as<Type, a_store_0>) {
			if(info) { tabs(); fputs("a_store_0\n", stderr); }
			reference ref = move(stack[--stack_size].get<reference>());
			local[0] = move(ref);
		}
		else if constexpr (same_as<Type, a_store_1>) {
			if(info) { tabs(); fputs("a_store_1\n", stderr); }
			reference ref = move(stack[--stack_size].get<reference>());
			local[1] = move(ref);
		}
		else if constexpr (same_as<Type, a_store_2>) {
			if(info) { tabs(); fputs("a_store_2\n", stderr); }
			reference ref = move(stack[--stack_size].get<reference>());
			local[2] = move(ref);
		}
		else if constexpr (same_as<Type, a_store_3>) {
			if(info) { tabs(); fputs("a_store_3\n", stderr); }
			reference ref = move(stack[--stack_size].get<reference>());
			local[3] = move(ref);
		}
		else if constexpr (same_as<Type, i_a_store>) {
			if(info) { tabs(); fputs("i_a_store\n", stderr); }
			int32 value = stack[--stack_size].get<jint>();
			return view_array.template operator()<int32>([&](int32& v) {
				v = value;
			});
		}
		else if constexpr (same_as<Type, a_a_store>) {
			if(info) { tabs(); fputs("a_a_store\n", stderr); }
			reference value = move(stack[--stack_size].get<reference>());
			return view_array.template operator()<reference>([&](reference& v) {
				v = value;
			});
		}
		else if constexpr (same_as<Type, b_a_store>) {
			if(info) { tabs(); fputs("b_a_store\n", stderr); }
			int32 value = stack[--stack_size].get<jint>();
			return view_array.template operator()<int8>([&](int8& v) {
				v = (int8) value;
			});
		}
		else if constexpr (same_as<Type, c_a_store>) {
			if(info) { tabs(); fputs("c_a_store\n", stderr); }
			int32 value = stack[--stack_size].get<jint>();
			return view_array.template operator() <int16>([&](int16& v) {
				v = (int16) value;
			});
		}

		else if constexpr (same_as<Type, pop>) {
			if(info) { tabs(); fputs("pop\n", stderr); }
			--stack_size;
		}
		else if constexpr (same_as<Type, dup>) {
			if(info) { tabs(); fputs("dup\n", stderr); }
			auto& value = stack[stack_size - 1];
			stack[stack_size++] = value;
		}
		else if constexpr (same_as<Type, dup_x1>) {
			if(info) { tabs(); fputs("dup_x1\n", stderr); }
			stack_entry value1 = move(stack[--stack_size]);
			stack_entry value2 = move(stack[--stack_size]);
			stack[stack_size++] = value1;
			stack[stack_size++] = move(value2);
			stack[stack_size++] = move(value1);
		}
		else if constexpr (same_as<Type, dup_2>) {
			if(info) { tabs(); fputs("dup_2\n", stderr); }
			if(
				stack[stack_size - 1].is<jlong>() ||
				stack[stack_size - 1].is<jdouble>()
			) {
				auto& value = stack[stack_size - 1];
				stack[stack_size++] = value;
			}
			else {
				auto& value_2 = stack[stack_size - 2];
				auto& value_1 = stack[stack_size - 1];
				stack[stack_size++] = value_2;
				stack[stack_size++] = value_1;
			}
		}

		else if constexpr (same_as<Type, i_add>) {
			if(info) { tabs(); fputs("i_add\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1 + value2 };
		}
		else if constexpr (same_as<Type, l_add>) {
			if(info) { tabs(); fputs("l_add\n", stderr); }
			int64 value2 = stack[--stack_size].get<jlong>();
			int64 value1 = stack[--stack_size].get<jlong>();
			stack[stack_size++] = jlong{ value1 + value2 };
		}
		else if constexpr (same_as<Type, f_add>) {
			if(info) { tabs(); fputs("f_add\n", stderr); }
			float value2 = stack[--stack_size].get<jfloat>();
			float value1 = stack[--stack_size].get<jfloat>();
			stack[stack_size++] = jfloat{ value1 + value2 };
		}
		else if constexpr (same_as<Type, i_sub>) {
			if(info) { tabs(); fputs("i_sub\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1 - value2 };
		}
		else if constexpr (same_as<Type, l_sub>) {
			if(info) { tabs(); fputs("l_sub\n", stderr); }
			int64 value2 = stack[--stack_size].get<jlong>();
			int64 value1 = stack[--stack_size].get<jlong>();
			stack[stack_size++] = jlong{ value1 - value2 };
		}
		else if constexpr (same_as<Type, i_mul>) {
			if(info) { tabs(); fputs("i_mul\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1 * value2 };
		}
		else if constexpr (same_as<Type, l_mul>) {
			if(info) { tabs(); fputs("l_mul\n", stderr); }
			int64 value2 = stack[--stack_size].get<jlong>();
			int64 value1 = stack[--stack_size].get<jlong>();
			stack[stack_size++] = jlong{ value1 * value2 };
		}
		else if constexpr (same_as<Type, f_mul>) {
			if(info) { tabs(); fputs("f_mul\n", stderr); }
			float value2 = stack[--stack_size].get<jfloat>();
			float value1 = stack[--stack_size].get<jfloat>();
			stack[stack_size++] = jfloat{ value1 * value2 };
		}
		else if constexpr (same_as<Type, i_div>) {
			if(info) { tabs(); fputs("i_div\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1 / value2 };
		}
		else if constexpr (same_as<Type, f_div>) {
			if(info) { tabs(); fputs("f_div\n", stderr); }
			float value2 = stack[--stack_size].get<jfloat>();
			float value1 = stack[--stack_size].get<jfloat>();
			stack[stack_size++] = jfloat{ value1 / value2 };
		}
		else if constexpr (same_as<Type, i_rem>) {
			if(info) { tabs(); fputs("i_rem\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1 - (value1 / value2) * value2 };
		}
		else if constexpr (same_as<Type, i_neg>) {
			if(info) { tabs(); fputs("i_neg\n", stderr); }
			int32 value = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ -value };
		}
		else if constexpr (same_as<Type, i_sh_l>) {
			if(info) { tabs(); fputs("i_sh_l\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1 << (value2 & 0x1F) };
		}
		else if constexpr (same_as<Type, l_sh_l>) {
			if(info) { tabs(); fputs("l_sh_l\n", stderr); }
			int64 value2 = stack[--stack_size].get<jlong>();
			int64 value1 = stack[--stack_size].get<jlong>();
			stack[stack_size++] = jlong{ value1 << (value2 & 0x3F) };
		}
		else if constexpr (same_as<Type, i_sh_r>) {
			if(info) { tabs(); fputs("i_sh_r\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1 >> (value2 & 0x1F) };
		}
		else if constexpr (same_as<Type, l_sh_r>) {
			if(info) { tabs(); fputs("l_sh_r\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int64 value1 = stack[--stack_size].get<jlong>();
			stack[stack_size++] = jlong{ value1 >> (value2 & 0x3F) };
		}
		else if constexpr (same_as<Type, i_u_sh_r>) {
			if(info) { tabs(); fputs("i_u_sh_r\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint {
				int32(uint32(value1) >> (value2 & 0x1F))
			};
		}
		else if constexpr (same_as<Type, l_u_sh_r>) {
			if(info) { tabs(); fputs("l_ush_r\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int64 value1 = stack[--stack_size].get<jlong>();
			stack[stack_size++] = jlong {
				int64(uint64(value1) >> (value2 & 0x3F))
			};
		}
		else if constexpr (same_as<Type, i_and>) {
			if(info) { tabs(); fputs("i_and\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1 & value2 };
		}
		else if constexpr (same_as<Type, l_and>) {
			if(info) { tabs(); fputs("l_and\n", stderr); }
			int64 value2 = stack[--stack_size].get<jlong>();
			int64 value1 = stack[--stack_size].get<jlong>();
			stack[stack_size++] = jlong{ value1 & value2 };
		}
		else if constexpr (same_as<Type, i_or>) {
			if(info) { tabs(); fputs("i_or\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1 | value2 };
		}
		else if constexpr (same_as<Type, l_or>) {
			if(info) { tabs(); fputs("l_or\n", stderr); }
			int64 value2 = stack[--stack_size].get<jlong>();
			int64 value1 = stack[--stack_size].get<jlong>();
			stack[stack_size++] = jlong{ value1 | value2 };
		}
		else if constexpr (same_as<Type, i_xor>) {
			if(info) { tabs(); fputs("i_xor\n", stderr); }
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1 ^ value2 };
		}
		else if constexpr (same_as<Type, i_inc>) {
			if(info) {
				tabs(); fprintf(stderr, "i_inc %hhu %hhd\n", x.index, x.value);
			}
			local[x.index].template get<jint>() += x.value;
		}
		else if constexpr (same_as<Type, i_to_l>) {
			if(info) { tabs(); fputs("i_to_l\n", stderr); }
			int32 value = stack[stack_size - 1].get<jint>();
			stack[stack_size - 1] = jlong{ value };
		}
		else if constexpr (same_as<Type, i_to_f>) {
			if(info) { tabs(); fputs("i_to_f\n", stderr); }
			int32 value = stack[stack_size - 1].get<jint>();
			stack[stack_size - 1] = jfloat{ (float) value };
		}
		else if constexpr (same_as<Type, l_to_i>) {
			if(info) { tabs(); fputs("l_to_i\n", stderr); }
			int64 value = stack[stack_size - 1].get<jlong>();
			stack[stack_size - 1] = jint{ (int32) (value & 0xFFFFFFFF) };
		}
		else if constexpr (same_as<Type, f_to_i>) {
			if(info) { tabs(); fputs("f_to_i\n", stderr); }
			float value = stack[stack_size - 1].get<jfloat>();
			int32 result;
			if(isnan(value)) {
				result = 0;
			}
			else {
				result = (int32) value;
			}
			if(value)
			stack[stack_size - 1] = jint{ result };
		}
		else if constexpr (same_as<Type, i_to_b>) {
			if(info) { tabs(); fputs("i_to_b\n", stderr); }
			int32 value = stack[stack_size - 1].get<jint>();
			stack[stack_size - 1] = jint{ (int8) value };
		}
		else if constexpr (same_as<Type, i_to_c>) {
			if(info) { tabs(); fputs("i_to_c\n", stderr); }
			int32 value = stack[stack_size - 1].get<jint>();
			stack[stack_size - 1] = jint{ (int16) value };
		}

		else if constexpr (same_as<Type, l_cmp>) {
			if(info) { tabs(); fputs("l_cmp\n", stderr); }
			int64 value_2 = stack[--stack_size].get<jlong>();
			int64 value_1 = stack[--stack_size].get<jlong>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			if(value_1 == value_2) result =  0;
			if(value_1 <  value_2) result = -1;
			stack[stack_size++] = jint{ result };
		}
		else if constexpr (same_as<Type, f_cmp_l>) {
			if(info) { tabs(); fputs("f_cmp_l\n", stderr); }
			float value_2 = stack[--stack_size].get<jfloat>();
			float value_1 = stack[--stack_size].get<jfloat>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			else if(value_1 == value_2) result =  0;
			else if(value_1 <  value_2) result = -1;
			else {
				result = -1;
			}
			stack[stack_size++] = jint{ result };
		}
		else if constexpr (same_as<Type, f_cmp_g>) {
			if(info) { tabs(); fputs("f_cmp_g\n", stderr); }
			float value_2 = stack[--stack_size].get<jfloat>();
			float value_1 = stack[--stack_size].get<jfloat>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			else if(value_1 == value_2) result =  0;
			else if(value_1 <  value_2) result = -1;
			else {
				result = 1;
			}
			stack[stack_size++] = jint{ result };
		}
		else if constexpr (same_as<Type, if_eq>) {
			if(info) {
				tabs(); fputs("if_eq ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack[--stack_size].get<jint>();
			if(value == 0) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_ne>) {
			if(info) {
				tabs(); fputs("if_ne ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack[--stack_size].get<jint>();
			if(value != 0) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_lt>) {
			if(info) {
				tabs(); fputs("if_lt ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack[--stack_size].get<jint>();
			if(value < 0) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_ge>) {
			if(info) {
				tabs(); fputs("if_ge ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack[--stack_size].get<jint>();
			if(value >= 0) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_gt>) {
			if(info) {
				tabs(); fputs("if_gt ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack[--stack_size].get<jint>();
			if(value > 0) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_le>) {
			if(info) {
				tabs(); fputs("if_le ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack[--stack_size].get<jint>();
			if(value <= 0) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_eq>) {
			if(info) {
				tabs(); fputs("if_i_cmp_eq ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			if(value1 == value2) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_ne>) {
			if(info) {
				tabs(); fputs("if_i_cmp_ne ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			if(value1 != value2) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_lt>) {
			if(info) {
				tabs(); fputs("if_i_cmp_lt ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			if(value1 < value2) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_ge>) {
			if(info) {
				tabs(); fputs("if_i_cmp_ge ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			if(value1 >= value2) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_gt>) {
			if(info) {
				tabs(); fputs("if_i_cmp_gt ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			if(value1 > value2) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_le>) {
			if(info) {
				tabs(); fputs("if_i_cmp_le ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack[--stack_size].get<jint>();
			int32 value1 = stack[--stack_size].get<jint>();
			if(value1 <= value2) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_a_cmp_eq>) {
			if(info) {
				tabs(); fputs("if_a_cmp_eq ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			reference& value2 = stack[--stack_size].get<reference>();
			reference& value1 = stack[--stack_size].get<reference>();
			if(value1.object_ptr() == value2.object_ptr()) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_a_cmp_ne>) {
			if(info) {
				tabs(); fputs("if_a_cmp_ne ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			reference& value2 = stack[--stack_size].get<reference>();
			reference& value1 = stack[--stack_size].get<reference>();
			if(value1.object_ptr() != value2.object_ptr()) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, go_to>) {
			if(info) {
				tabs(); fputs("go_to ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			it = m.code().begin() + pc + x.branch;
		}
		else if constexpr (same_as<Type, i_return>) {
			if(info) { tabs(); fputs("i_return\n", stderr); }
			result = stack[--stack_size].get<jint>();
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, l_return>) {
			if(info) { tabs(); fputs("l_return\n", stderr); }
			result = stack[--stack_size].get<jlong>();
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, d_return>) {
			if(info) { tabs(); fputs("d_return\n", stderr); }
			result = stack[--stack_size].get<jdouble>();
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, a_return>) {
			if(info) { tabs(); fputs("a_return\n", stderr); }
			result = move(stack[--stack_size].get<reference>());
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, _return>) {
			if(info) { tabs(); fputs("return\n", stderr); }
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, get_static>) {
			if(info) {
				tabs(); fputs("get_static ", stderr);
				cc::field_ref field_ref = c.field_ref_constant(x.index);
				cc::name_and_type nat {
					c.name_and_type_constant(field_ref.name_and_type_index)
				};
				cc::utf8 name = c.utf8_constant(nat.name_index);
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}
			static_field_with_class sfwc = c.get_static_field(x.index);
			field_value& value = sfwc.static_field().value();
			stack[stack_size++] = get_field_value(value);
		}
		else if constexpr (same_as<Type, put_static>) {
			if(info) {
				tabs(); fputs("put_static ", stderr);
				fprintf(stderr, "%hd\n", x.index);
			}
			static_field_with_class sfwc = c.get_static_field(x.index);
			field_value& static_field_value = sfwc.static_field().value();
			stack_entry value = move(stack[--stack_size]);
			put_field_value(static_field_value, move(value));
		}
		else if constexpr (same_as<Type, get_field>) {
			if(info) {
				tabs(); fputs("get_field ", stderr);
				auto field_ref = c.field_ref_constant(x.index);
				cc::name_and_type nat = c.name_and_type_constant(
					field_ref.name_and_type_index
				);
				cc::_class class_ = c.class_constant(field_ref.class_index);
				cc::utf8 class_name = c.utf8_constant(class_.name_index);
				cc::utf8 field_name = c.utf8_constant(nat.name_index);
				fwrite(class_name.data(), 1, class_name.size(), stderr);
				fputc('.', stderr);
				fwrite(field_name.data(), 1, field_name.size(), stderr);
				fputc('\n', stderr);
			}

			instance_field_index instance_field_index {
				c.get_resolved_instance_field_index(x.index)
			};

			reference ref = move(stack[--stack_size].get<reference>());
			field_value& value = ref.object()[instance_field_index];
			stack[stack_size++] = get_field_value(value);
		}
		else if constexpr (same_as<Type, put_field>) {
			if(info) {
				tabs(); fputs("put_field ", stderr);
				cc::field_ref field_ref = c.field_ref_constant(x.index);
				cc::name_and_type nat = c.name_and_type_constant(
					field_ref.name_and_type_index
				);
				cc::_class class_ = c.class_constant(field_ref.class_index);
				cc::utf8 class_name = c.utf8_constant(class_.name_index);
				cc::utf8 name = c.utf8_constant(nat.name_index);
				fwrite(class_name.data(), 1, class_name.size(), stderr);
				fputc('.', stderr);
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			auto instance_field_index {
				c.get_resolved_instance_field_index(x.index)
			};

			stack_entry value = move(stack[--stack_size]);
			reference ref = move(stack[--stack_size].get<reference>());
			field_value& to = ref.object()[instance_field_index];
			put_field_value(to, move(value));
		}
		else if constexpr (same_as<Type, instr::invoke_virtual>) {
			::invoke_virtual(
				method_ref_index{ x.index }, c, stack, stack_size
			);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_special>) {
			auto possible_exception = ::invoke_special(
				method_ref_index{ x.index }, c, stack, stack_size
			);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_static>) {
			auto possible_exception = ::invoke_static(
				method_ref_index{ x.index }, c, stack, stack_size
			);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_interface>) {
			auto possible_exception = ::invoke_interface(
				method_ref_index{ x.index }, arguments_count{ x.count },
				c, stack, stack_size
			);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_dynamic>) {

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

			if(info) {
				tabs(); fputs("a_new_array ", stderr);
				auto name = element_class.name();
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			int32 count = stack[--stack_size].get<jint>();

			auto ref = create_array_of(element_class, count);
			stack[stack_size++] = move(ref);
		}
		else if constexpr (same_as<Type, instr::array_length>) {
			if(info) { tabs(); fputs("array_length\n", stderr); }
			reference ref = stack[--stack_size].get<reference>();
			if(ref.is_null()) {
				thrown = create_null_pointer_exception();
				return handle_thrown();
			}
			stack[stack_size++] = jint{ ::array_length(ref.object()) };
		}
		else if constexpr (same_as<Type, a_throw>) {
			if(info) { tabs(); fputs("a_throw\n", stderr); }

			reference ref = move(stack[--stack_size].get<reference>());
			if(ref.is_null()) {
				ref = create_null_pointer_exception();
			}
			thrown = move(ref);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, check_cast>) {
			if(info) {
				tabs(); fputs("check_cast ", stderr);
				fprintf(stderr, "%hu\n", x.index);
			}
			//_class& type = c.get_class(x.index);
			//reference objectref = stack[stack_size - 1].get<reference>();
			// TODO
		}
		else if constexpr (same_as<Type, instance_of>) {
			if(info) {
				cc::_class _class = c.class_constant(x.index);
				cc::utf8 name = c.utf8_constant(_class.name_index);
				tabs(); fputs("instance_of ", stderr);
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			reference objectref = move(stack[--stack_size].get<reference>());
			_class& s = objectref.object()._class();

			int32 result = 0;
			if(!objectref.is_null()) {
				_class& t = c.get_class(x.index);

				if(!s.is_interface()) {
					if(!t.is_interface()) {
						result = &s == &t || s.is_subclass_of(t);
					}
					else {
						result = s.is_implements(t);
					}
				}
				else {
					abort();//TODO
				}
			}
			stack[stack_size++] = jint{ result };
		}
		else if constexpr (same_as<Type, if_null>) {
			if(info) {
				tabs(); fputs("if_null ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			reference ref = move(stack[--stack_size].get<reference>());
			if(ref.is_null()) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_non_null>) {
			if(info) {
				tabs(); fputs("if_non_null ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			reference ref = move(stack[--stack_size].get<reference>());
			if(!ref.is_null()) {
				it = m.code().begin() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, uint8>) {
			if(info) tabs();
			fprintf(stderr, "unknown instruction ");
			for_each_digit_in_number(
				number{ x }, base{ 10 },
				[](auto digit) { fputc('0' + digit, stderr); }
			);
			abort();
		}
		else {
			on_unimplemented_instruction(Type::code);
		}

		return loop_action::next;

	}, m.code().size());

	return result;
}