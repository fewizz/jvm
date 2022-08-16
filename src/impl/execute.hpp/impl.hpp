#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/latest_context.hpp"
#include "decl/thrown.hpp"
#include "decl/array.hpp"
#include "decl/object/create.hpp"
#include "decl/native/functions.hpp"
#include "decl/abort.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/lib/java/lang/index_out_of_bounds_exception.hpp"

#include "./get_field_value.hpp"
#include "./put_field_value.hpp"
#include "./ldc.hpp"
#include "./invoke_dynamic.hpp"
#include "./invoke_virtual.hpp"
#include "./invoke_special.hpp"
#include "./invoke_static.hpp"
#include "./invoke_interface.hpp"
#include "./new_array.hpp"

#include <class_file/reader.hpp>
#include <class_file/descriptor/reader.hpp>
#include <class_file/attribute/code/reader.hpp>

#include <number.hpp>
#include <c_string.hpp>
#include <on_scope_exit.hpp>
#include <max.hpp>
#include <range.hpp>

#include <stdio.h>
#include <math.h>

static stack_entry execute(
	method& m,
	arguments_span args
) {
	namespace cf = class_file;
	namespace instr = cf::attribute::code::instruction;

	_class& c = m._class();
	if(info) {
		tabs();
		fputs("executing: ", stderr);
		fwrite(c.name().elements_ptr(), 1, c.name().size(), stderr);
		fputc('.', stderr);
		fwrite(m.name().elements_ptr(), 1, m.name().size(), stderr);
		fwrite(m.descriptor().elements_ptr(), 1, m.descriptor().size(), stderr);
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
		if(!m.native_function_is_loaded()) {
			//m.native_function(native_functions.find(mwc));
		}
		auto& native_function = m.native_function();
		return native_function.call(args);
	}

	if(m.code().elements_ptr() == nullptr) {
		fputs("no code", stderr); abort();
	}

	cf::attribute::code::reader<
		uint8*,
		cf::attribute::code::reader_stage::code
	> reader{ m.code().elements_ptr() };

	nuint stack_size = (nuint) max(m.code().max_stack, 1) * sizeof(stack_entry);
	alignas(stack_entry) uint8 stack_storage[stack_size];
	::stack stack{ memory_span{ stack_storage, stack_size } };

	stack_entry local[
		max(m.code().max_locals * 2, 1)
	]; // there may be longs or doubles

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
			code, number_base{ 10 },
			[](auto digit) { fputc('0' + digit, stderr); }
		);
		abort();
	};

	reader([&]<typename Type>(Type x, uint8*& it) {
		on_scope_exit update_pc{[&](){
			pc = it - m.code().iterator();
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
				bool subclass = thrown_class.is_sub_of(catch_class);

				if(!(same || subclass)) {
					continue;
				}

				it = m.code().iterator() + handler.handler_pc;
				stack.clear();
				stack.emplace_back(move(thrown));
				return loop_action::next;
			}

			return loop_action::stop;
		};

		auto view_array = [&]<typename E, typename Handler>(
			Handler&& handler
		) {
			int32 element_index = stack.pop_back().get<jint>();
			reference array_ref = stack.pop_back().get<reference>();
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
			stack.emplace_back(reference{});
		}
		else if constexpr (same_as<Type, i_const_m1>) {
			if(info) { tabs(); fputs("i_const_m1\n", stderr); }
			stack.emplace_back(jint{ -1 });
		}
		else if constexpr (same_as<Type, i_const_0>) {
			if(info) { tabs(); fputs("i_const_0\n", stderr); }
			stack.emplace_back(jint{ 0 });
		}
		else if constexpr (same_as<Type, i_const_1>) {
			if(info) { tabs(); fputs("i_const_1\n", stderr); }
			stack.emplace_back(jint{ 1 });
		}
		else if constexpr (same_as<Type, i_const_2>) {
			if(info) { tabs(); fputs("i_const_2\n", stderr); }
			stack.emplace_back(jint{ 2 });
		}
		else if constexpr (same_as<Type, i_const_3>) {
			if(info) { tabs(); fputs("i_const_3\n", stderr); }
			stack.emplace_back(jint{ 3 });
		}
		else if constexpr (same_as<Type, i_const_4>) {
			if(info) { tabs(); fputs("i_const_4\n", stderr); }
			stack.emplace_back(jint{ 4 });
		}
		else if constexpr (same_as<Type, i_const_5>) {
			if(info) { tabs(); fputs("i_const_5\n", stderr); }
			stack.emplace_back(jint{ 5 });
		}
		else if constexpr (same_as<Type, l_const_0>) {
			if(info) { tabs(); fputs("l_const_0\n", stderr); }
			stack.emplace_back(jlong{ 0 });
		}
		else if constexpr (same_as<Type, l_const_1>) {
			if(info) { tabs(); fputs("l_const_1\n", stderr); }
			stack.emplace_back(jlong{ 1 });
		}
		else if constexpr (same_as<Type, f_const_0>) {
			if(info) { tabs(); fputs("f_const_0\n", stderr); }
			stack.emplace_back(jfloat{ 0.0F });
		}
		else if constexpr (same_as<Type, f_const_1>) {
			if(info) { tabs(); fputs("f_const_1\n", stderr); }
			stack.emplace_back(jfloat{ 1.0F });
		}
		else if constexpr (same_as<Type, f_const_2>) {
			if(info) { tabs(); fputs("f_const_2\n", stderr); }
			stack.emplace_back(jfloat{ 2.0F });
		}
		else if constexpr (same_as<Type, bi_push>) {
			if(info) {
				tabs(); fputs("bi_push ", stderr);
				fprintf(stderr, "%hhd\n", x.value);
			}
			stack.emplace_back(jint{ x.value });
		}
		else if constexpr (same_as<Type, si_push>) {
			if(info) {
				tabs(); fputs("si_push ", stderr);
				fprintf(stderr, "%hd\n", x.value);
			}
			stack.emplace_back(jint{ x.value });
		}
		else if constexpr (
			same_as<Type, instr::ldc> || same_as<Type, instr::ldc_w>
		) {
			::ldc(x.index, c, stack);
		}
		else if constexpr (same_as<Type, instr::ldc_2_w>) {
			::ldc_2_w(x.index, c, stack);
		}

		else if constexpr (same_as<Type, i_load>) {
			if(info) {
				tabs(); fputs("i_load ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			stack.emplace_back(local[x.index].template get<jint>());
		}
		else if constexpr (same_as<Type, l_load>) {
			if(info) {
				tabs(); fputs("l_load ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			stack.emplace_back(local[x.index].template get<jlong>());
		}
		else if constexpr (same_as<Type, f_load>) {
			if(info) {
				tabs(); fputs("f_load ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			stack.emplace_back(local[x.index].template get<jfloat>());
		}
		else if constexpr (same_as<Type, a_load>) {
			if(info) {
				tabs(); fputs("a_load ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			stack.emplace_back(local[x.index].template get<reference>());
		}
		else if constexpr (same_as<Type, i_load_0>) {
			if(info) { tabs(); fputs("i_load_0\n", stderr); }
			stack.emplace_back(local[0].get<jint>());
		}
		else if constexpr (same_as<Type, i_load_1>) {
			if(info) { tabs(); fputs("i_load_1\n", stderr); }
			stack.emplace_back(local[1].get<jint>());
		}
		else if constexpr (same_as<Type, i_load_2>) {
			if(info) { tabs(); fputs("i_load_2\n", stderr); }
			stack.emplace_back(local[2].get<jint>());
		}
		else if constexpr (same_as<Type, i_load_3>) {
			if(info) { tabs(); fputs("i_load_3\n", stderr); }
			stack.emplace_back(local[3].get<jint>());
		}
		else if constexpr (same_as<Type, l_load_0>) {
			if(info) { tabs(); fputs("l_load_0\n", stderr); }
			stack.emplace_back(local[0].get<jlong>());
		}
		else if constexpr (same_as<Type, l_load_1>) {
			if(info) { tabs(); fputs("l_load_1\n", stderr); }
			stack.emplace_back(local[1].get<jlong>());
		}
		else if constexpr (same_as<Type, l_load_2>) {
			if(info) { tabs(); fputs("l_load_2\n", stderr); }
			stack.emplace_back(local[2].get<jlong>());
		}
		else if constexpr (same_as<Type, l_load_3>) {
			if(info) { tabs(); fputs("l_load_3\n", stderr); }
			stack.emplace_back(local[3].get<jlong>());
		}
		else if constexpr (same_as<Type, a_load_0>) {
			if(info) { tabs(); fputs("a_load_0\n", stderr); }
			reference ref = local[0].get<reference>();
			stack.emplace_back(move(ref));
		}
		else if constexpr (same_as<Type, a_load_1>) {
			if(info) { tabs(); fputs("a_load_1\n", stderr); }
			reference ref = local[1].get<reference>();
			stack.emplace_back(move(ref));
		}
		else if constexpr (same_as<Type, a_load_2>) {
			if(info) { tabs(); fputs("a_load_2\n", stderr); }
			reference ref = local[2].get<reference>();
			stack.emplace_back(move(ref));
		}
		else if constexpr (same_as<Type, a_load_3>) {
			if(info) { tabs(); fputs("a_load_3\n", stderr); }
			reference ref = local[3].get<reference>();
			stack.emplace_back(move(ref));
		}
		else if constexpr (same_as<Type, i_a_load>) {
			if(info) { tabs(); fputs("i_a_load\n", stderr); }
			return view_array.template operator()<int32>([&](int32& v) {
				stack.emplace_back(jint{ v });
			});
		}
		else if constexpr (same_as<Type, a_a_load>) {
			if(info) { tabs(); fputs("a_a_load\n", stderr); }
			return view_array.template operator()<reference>([&](reference& v) {
				stack.emplace_back(v);
			});
		}
		else if constexpr (same_as<Type, b_a_load>) {
			if(info) { tabs(); fputs("b_a_load\n", stderr); }
			return view_array.template operator()<int8>([&](int8& v) {
				stack.emplace_back(jint{ v });
			});
		}
		else if constexpr (same_as<Type, i_store>) {
			if(info) {
				tabs(); fputs("i_store ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			local[x.index] = stack.pop_back().get<jint>();
		}
		else if constexpr (same_as<Type, l_store>) {
			if(info) {
				tabs(); fputs("l_store ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			local[x.index] = stack.pop_back().get<jlong>();
		}
		else if constexpr (same_as<Type, f_store>) {
			if(info) {
				tabs(); fputs("f_store ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			local[x.index] = stack.pop_back().get<jfloat>();
		}
		else if constexpr (same_as<Type, a_store>) {
			if(info) {
				tabs(); fputs("a_store ", stderr);
				fprintf(stderr, "%hhu\n", x.index);
			}
			local[x.index] = move(stack.pop_back().get<reference>());
		}
		else if constexpr (same_as<Type, i_store_0>) {
			if(info) { tabs(); fputs("i_store_0\n", stderr); }
			local[0] = stack.pop_back().get<jint>();
		}
		else if constexpr (same_as<Type, i_store_1>) {
			if(info) { tabs(); fputs("i_store_1\n", stderr); }
			local[1] = stack.pop_back().get<jint>();
		}
		else if constexpr (same_as<Type, i_store_2>) {
			if(info) { tabs(); fputs("i_store_2\n", stderr); }
			local[2] = stack.pop_back().get<jint>();
		}
		else if constexpr (same_as<Type, i_store_3>) {
			if(info) { tabs(); fputs("i_store_3\n", stderr); }
			local[3] = stack.pop_back().get<jint>();
		}
		else if constexpr (same_as<Type, l_store_0>) {
			if(info) { tabs(); fputs("l_store_0\n", stderr); }
			local[0] = stack.pop_back().get<jlong>();
		}
		else if constexpr (same_as<Type, l_store_1>) {
			if(info) { tabs(); fputs("l_store_1\n", stderr); }
			local[1] = stack.pop_back().get<jlong>();
		}
		else if constexpr (same_as<Type, l_store_2>) {
			if(info) { tabs(); fputs("l_store_2\n", stderr); }
			local[2] = stack.pop_back().get<jlong>();
		}
		else if constexpr (same_as<Type, l_store_3>) {
			if(info) { tabs(); fputs("l_store_3\n", stderr); }
			local[3] = stack.pop_back().get<jlong>();
		}
		else if constexpr (same_as<Type, a_store_0>) {
			if(info) { tabs(); fputs("a_store_0\n", stderr); }
			reference ref = stack.pop_back().get<reference>();
			local[0] = move(ref);
		}
		else if constexpr (same_as<Type, a_store_1>) {
			if(info) { tabs(); fputs("a_store_1\n", stderr); }
			reference ref = stack.pop_back().get<reference>();
			local[1] = move(ref);
		}
		else if constexpr (same_as<Type, a_store_2>) {
			if(info) { tabs(); fputs("a_store_2\n", stderr); }
			reference ref = stack.pop_back().get<reference>();
			local[2] = move(ref);
		}
		else if constexpr (same_as<Type, a_store_3>) {
			if(info) { tabs(); fputs("a_store_3\n", stderr); }
			reference ref = stack.pop_back().get<reference>();
			local[3] = move(ref);
		}
		else if constexpr (same_as<Type, i_a_store>) {
			if(info) { tabs(); fputs("i_a_store\n", stderr); }
			int32 value = stack.pop_back().get<jint>();
			return view_array.template operator()<int32>([&](int32& v) {
				v = value;
			});
		}
		else if constexpr (same_as<Type, a_a_store>) {
			if(info) { tabs(); fputs("a_a_store\n", stderr); }
			reference value = stack.pop_back().get<reference>();
			return view_array.template operator()<reference>([&](reference& v) {
				v = value;
			});
		}
		else if constexpr (same_as<Type, b_a_store>) {
			if(info) { tabs(); fputs("b_a_store\n", stderr); }
			int32 value = stack.pop_back().get<jint>();
			return view_array.template operator()<int8>([&](int8& v) {
				v = (int8) value;
			});
		}
		else if constexpr (same_as<Type, c_a_store>) {
			if(info) { tabs(); fputs("c_a_store\n", stderr); }
			int32 value = stack.pop_back().get<jint>();
			return view_array.template operator() <int16>([&](int16& v) {
				v = (int16) value;
			});
		}

		else if constexpr (same_as<Type, pop>) {
			if(info) { tabs(); fputs("pop\n", stderr); }
			stack.pop_back();
		}
		else if constexpr (same_as<Type, dup>) {
			if(info) { tabs(); fputs("dup\n", stderr); }
			auto& value = stack.back();
			stack.emplace_back(value);
		}
		else if constexpr (same_as<Type, dup_x1>) {
			if(info) { tabs(); fputs("dup_x1\n", stderr); }
			stack_entry value1 = stack.pop_back();
			stack_entry value2 = stack.pop_back();
			stack.emplace_back(value1);
			stack.emplace_back(move(value2));
			stack.emplace_back(move(value1));
		}
		else if constexpr (same_as<Type, dup_2>) {
			if(info) { tabs(); fputs("dup_2\n", stderr); }
			if(
				stack.back().is<jlong>() ||
				stack.back().is<jdouble>()
			) {
				auto& value = stack.back();
				stack.emplace_back(value);
			}
			else {
				auto& value_2 = stack[stack.size() - 2];
				auto& value_1 = stack[stack.size() - 1];
				stack.emplace_back(value_2);
				stack.emplace_back(value_1);
			}
		}

		else if constexpr (same_as<Type, i_add>) {
			if(info) { tabs(); fputs("i_add\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			stack.emplace_back(jint{ value1 + value2 });
		}
		else if constexpr (same_as<Type, l_add>) {
			if(info) { tabs(); fputs("l_add\n", stderr); }
			int64 value2 = stack.pop_back().get<jlong>();
			int64 value1 = stack.pop_back().get<jlong>();
			stack.emplace_back(jlong{ value1 + value2 });
		}
		else if constexpr (same_as<Type, f_add>) {
			if(info) { tabs(); fputs("f_add\n", stderr); }
			float value2 = stack.pop_back().get<jfloat>();
			float value1 = stack.pop_back().get<jfloat>();
			stack.emplace_back(jfloat{ value1 + value2 });
		}
		else if constexpr (same_as<Type, i_sub>) {
			if(info) { tabs(); fputs("i_sub\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			stack.emplace_back(jint{ value1 - value2 });
		}
		else if constexpr (same_as<Type, l_sub>) {
			if(info) { tabs(); fputs("l_sub\n", stderr); }
			int64 value2 = stack.pop_back().get<jlong>();
			int64 value1 = stack.pop_back().get<jlong>();
			stack.emplace_back(jlong{ value1 - value2 });
		}
		else if constexpr (same_as<Type, i_mul>) {
			if(info) { tabs(); fputs("i_mul\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			stack.emplace_back(jint{ value1 * value2 });
		}
		else if constexpr (same_as<Type, l_mul>) {
			if(info) { tabs(); fputs("l_mul\n", stderr); }
			int64 value2 = stack.pop_back().get<jlong>();
			int64 value1 = stack.pop_back().get<jlong>();
			stack.emplace_back(jlong{ value1 * value2 });
		}
		else if constexpr (same_as<Type, f_mul>) {
			if(info) { tabs(); fputs("f_mul\n", stderr); }
			float value2 = stack.pop_back().get<jfloat>();
			float value1 = stack.pop_back().get<jfloat>();
			stack.emplace_back(jfloat{ value1 * value2 });
		}
		else if constexpr (same_as<Type, i_div>) {
			if(info) { tabs(); fputs("i_div\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			stack.emplace_back(jint{ value1 / value2 });
		}
		else if constexpr (same_as<Type, f_div>) {
			if(info) { tabs(); fputs("f_div\n", stderr); }
			float value2 = stack.pop_back().get<jfloat>();
			float value1 = stack.pop_back().get<jfloat>();
			stack.emplace_back(jfloat{ value1 / value2 });
		}
		else if constexpr (same_as<Type, i_rem>) {
			if(info) { tabs(); fputs("i_rem\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			stack.emplace_back(jint{ value1 - (value1 / value2) * value2 });
		}
		else if constexpr (same_as<Type, i_neg>) {
			if(info) { tabs(); fputs("i_neg\n", stderr); }
			int32 value = stack.pop_back().get<jint>();
			stack.emplace_back(jint{ -value });
		}
		else if constexpr (same_as<Type, i_sh_l>) {
			if(info) { tabs(); fputs("i_sh_l\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			stack.emplace_back(jint{ value1 << (value2 & 0x1F) });
		}
		else if constexpr (same_as<Type, l_sh_l>) {
			if(info) { tabs(); fputs("l_sh_l\n", stderr); }
			int64 value2 = stack.pop_back().get<jlong>();
			int64 value1 = stack.pop_back().get<jlong>();
			stack.emplace_back(jlong{ value1 << (value2 & 0x3F) });
		}
		else if constexpr (same_as<Type, i_sh_r>) {
			if(info) { tabs(); fputs("i_sh_r\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			stack.emplace_back(jint{ value1 >> (value2 & 0x1F) });
		}
		else if constexpr (same_as<Type, l_sh_r>) {
			if(info) { tabs(); fputs("l_sh_r\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int64 value1 = stack.pop_back().get<jlong>();
			stack.emplace_back(jlong{ value1 >> (value2 & 0x3F) });
		}
		else if constexpr (same_as<Type, i_u_sh_r>) {
			if(info) { tabs(); fputs("i_u_sh_r\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			stack.emplace_back(jint {
				int32(uint32(value1) >> (value2 & 0x1F))
			});
		}
		else if constexpr (same_as<Type, l_u_sh_r>) {
			if(info) { tabs(); fputs("l_ush_r\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int64 value1 = stack.pop_back().get<jlong>();
			stack.emplace_back(jlong {
				int64(uint64(value1) >> (value2 & 0x3F))
			});
		}
		else if constexpr (same_as<Type, i_and>) {
			if(info) { tabs(); fputs("i_and\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			stack.emplace_back(jint{ value1 & value2 });
		}
		else if constexpr (same_as<Type, l_and>) {
			if(info) { tabs(); fputs("l_and\n", stderr); }
			int64 value2 = stack.pop_back().get<jlong>();
			int64 value1 = stack.pop_back().get<jlong>();
			stack.emplace_back(jlong{ value1 & value2 });
		}
		else if constexpr (same_as<Type, i_or>) {
			if(info) { tabs(); fputs("i_or\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			stack.emplace_back(jint{ value1 | value2 });
		}
		else if constexpr (same_as<Type, l_or>) {
			if(info) { tabs(); fputs("l_or\n", stderr); }
			int64 value2 = stack.pop_back().get<jlong>();
			int64 value1 = stack.pop_back().get<jlong>();
			stack.emplace_back(jlong{ value1 | value2 });
		}
		else if constexpr (same_as<Type, i_xor>) {
			if(info) { tabs(); fputs("i_xor\n", stderr); }
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			stack.emplace_back(jint{ value1 ^ value2 });
		}
		else if constexpr (same_as<Type, i_inc>) {
			if(info) {
				tabs(); fprintf(stderr, "i_inc %hhu %hhd\n", x.index, x.value);
			}
			local[x.index].template get<jint>() += x.value;
		}
		else if constexpr (same_as<Type, i_to_l>) {
			if(info) { tabs(); fputs("i_to_l\n", stderr); }
			int32 value = stack.back().get<jint>();
			stack.back() = jlong{ value };
		}
		else if constexpr (same_as<Type, i_to_f>) {
			if(info) { tabs(); fputs("i_to_f\n", stderr); }
			int32 value = stack.back().get<jint>();
			stack.back() = jfloat{ (float) value };
		}
		else if constexpr (same_as<Type, l_to_i>) {
			if(info) { tabs(); fputs("l_to_i\n", stderr); }
			int64 value = stack.back().get<jlong>();
			stack.back() = jint{ (int32) (value & 0xFFFFFFFF) };
		}
		else if constexpr (same_as<Type, f_to_i>) {
			if(info) { tabs(); fputs("f_to_i\n", stderr); }
			float value = stack.back().get<jfloat>();
			int32 result;
			if(isnan(value)) {
				result = 0;
			}
			else {
				result = (int32) value;
			}
			stack.back() = jint{ result };
		}
		else if constexpr (same_as<Type, i_to_b>) {
			if(info) { tabs(); fputs("i_to_b\n", stderr); }
			int32 value = stack.back().get<jint>();
			stack.back() = jint{ (int8) value };
		}
		else if constexpr (same_as<Type, i_to_c>) {
			if(info) { tabs(); fputs("i_to_c\n", stderr); }
			int32 value = stack.back().get<jint>();
			stack.back() = jint{ (int16) value };
		}

		else if constexpr (same_as<Type, l_cmp>) {
			if(info) { tabs(); fputs("l_cmp\n", stderr); }
			int64 value_2 = stack.pop_back().get<jlong>();
			int64 value_1 = stack.pop_back().get<jlong>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			if(value_1 == value_2) result =  0;
			if(value_1 <  value_2) result = -1;
			stack.emplace_back(jint{ result });
		}
		else if constexpr (same_as<Type, f_cmp_l>) {
			if(info) { tabs(); fputs("f_cmp_l\n", stderr); }
			float value_2 = stack.pop_back().get<jfloat>();
			float value_1 = stack.pop_back().get<jfloat>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			else if(value_1 == value_2) result =  0;
			else if(value_1 <  value_2) result = -1;
			else { // NaN
				result = -1;
			}
			stack.emplace_back(jint{ result });
		}
		else if constexpr (same_as<Type, f_cmp_g>) {
			if(info) { tabs(); fputs("f_cmp_g\n", stderr); }
			float value_2 = stack.pop_back().get<jfloat>();
			float value_1 = stack.pop_back().get<jfloat>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			else if(value_1 == value_2) result =  0;
			else if(value_1 <  value_2) result = -1;
			else { // NaN
				result = 1;
			}
			stack.emplace_back(jint{ result });
		}
		else if constexpr (same_as<Type, if_eq>) {
			if(info) {
				tabs(); fputs("if_eq ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack.pop_back().get<jint>();
			if(value == 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_ne>) {
			if(info) {
				tabs(); fputs("if_ne ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack.pop_back().get<jint>();
			if(value != 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_lt>) {
			if(info) {
				tabs(); fputs("if_lt ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack.pop_back().get<jint>();
			if(value < 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_ge>) {
			if(info) {
				tabs(); fputs("if_ge ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack.pop_back().get<jint>();
			if(value >= 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_gt>) {
			if(info) {
				tabs(); fputs("if_gt ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack.pop_back().get<jint>();
			if(value > 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_le>) {
			if(info) {
				tabs(); fputs("if_le ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value = stack.pop_back().get<jint>();
			if(value <= 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_eq>) {
			if(info) {
				tabs(); fputs("if_i_cmp_eq ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			if(value1 == value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_ne>) {
			if(info) {
				tabs(); fputs("if_i_cmp_ne ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			if(value1 != value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_lt>) {
			if(info) {
				tabs(); fputs("if_i_cmp_lt ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			if(value1 < value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_ge>) {
			if(info) {
				tabs(); fputs("if_i_cmp_ge ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			if(value1 >= value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_gt>) {
			if(info) {
				tabs(); fputs("if_i_cmp_gt ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			if(value1 > value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_le>) {
			if(info) {
				tabs(); fputs("if_i_cmp_le ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			int32 value2 = stack.pop_back().get<jint>();
			int32 value1 = stack.pop_back().get<jint>();
			if(value1 <= value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_a_cmp_eq>) {
			if(info) {
				tabs(); fputs("if_a_cmp_eq ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			reference value2 = stack.pop_back().get<reference>();
			reference value1 = stack.pop_back().get<reference>();
			if(value1.object_ptr() == value2.object_ptr()) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_a_cmp_ne>) {
			if(info) {
				tabs(); fputs("if_a_cmp_ne ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			reference value2 = stack.pop_back().get<reference>();
			reference value1 = stack.pop_back().get<reference>();
			if(value1.object_ptr() != value2.object_ptr()) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, go_to>) {
			if(info) {
				tabs(); fputs("go_to ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			it = m.code().iterator() + pc + x.branch;
		}
		else if constexpr (same_as<Type, i_return>) {
			if(info) { tabs(); fputs("i_return\n", stderr); }
			result = stack.pop_back().get<jint>();
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, l_return>) {
			if(info) { tabs(); fputs("l_return\n", stderr); }
			result = stack.pop_back().get<jlong>();
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, d_return>) {
			if(info) { tabs(); fputs("d_return\n", stderr); }
			result = stack.pop_back().get<jdouble>();
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, a_return>) {
			if(info) { tabs(); fputs("a_return\n", stderr); }
			result = move(stack.pop_back().get<reference>());
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
				fwrite(name.elements_ptr(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}
			field_value& value = c.get_static_field_value(x.index);
			stack.emplace_back(get_field_value(value));
		}
		else if constexpr (same_as<Type, put_static>) {
			if(info) {
				tabs(); fputs("put_static ", stderr);
				cc::field_ref field_ref = c.field_ref_constant(x.index);
				cc::name_and_type nat {
					c.name_and_type_constant(field_ref.name_and_type_index)
				};
				cc::utf8 name = c.utf8_constant(nat.name_index);
				fwrite(name.elements_ptr(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}
			field_value& field_value = c.get_static_field_value(x.index);
			stack_entry stack_value = stack.pop_back();
			put_field_value(field_value, move(stack_value));
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
				fwrite(class_name.elements_ptr(), 1, class_name.size(), stderr);
				fputc('.', stderr);
				fwrite(field_name.elements_ptr(), 1, field_name.size(), stderr);
				fputc('\n', stderr);
			}

			instance_field_index index {
				c.get_resolved_instance_field_index(x.index)
			};

			reference ref = stack.pop_back().get<reference>();
			field_value& value = ref.object()[index];
			stack.emplace_back(get_field_value(value));
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
				fwrite(class_name.elements_ptr(), 1, class_name.size(), stderr);
				fputc('.', stderr);
				fwrite(name.elements_ptr(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			instance_field_index index {
				c.get_resolved_instance_field_index(x.index)
			};

			stack_entry value = stack.pop_back();
			reference ref = stack.pop_back().get<reference>();
			if(ref.is_null()) {
				fputs("object containing field is null", stderr);
				abort();
			}
			field_value& to = ref.object()[index];
			put_field_value(to, move(value));
		}
		else if constexpr (same_as<Type, instr::invoke_virtual>) {
			::invoke_virtual(x.index, c, stack);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_special>) {
			::invoke_special(x.index, c, stack);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_static>) {
			::invoke_static(x.index, c, stack);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_interface>) {
			::invoke_interface(x.index, parameters_count{ x.count }, c, stack);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_dynamic>) {
			::invoke_dynamic(x.index, c, stack);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, _new>) {
			if(info) {
				tabs(); fputs("new ", stderr);
				cc::utf8 name = c.utf8_constant(
					c.class_constant(x.index).name_index
				);
				fwrite(name.elements_ptr(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}
			_class& c0 = c.get_class(x.index);
			stack.emplace_back(create_object(c0));
		}
		else if constexpr (same_as<Type, instr::new_array>) {
			::new_array(/* c, */ x.type, stack);
		}
		else if constexpr (same_as<Type, a_new_array>) {
			_class& element_class = c.get_class(x.index);

			if(info) {
				tabs(); fputs("a_new_array ", stderr);
				cc::utf8 name = element_class.name();
				fwrite(name.elements_ptr(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			int32 count = stack.pop_back().get<jint>();

			auto ref = create_array_of(element_class, count);
			stack.emplace_back(move(ref));
		}
		else if constexpr (same_as<Type, instr::array_length>) {
			if(info) { tabs(); fputs("array_length\n", stderr); }
			reference ref = stack.pop_back().get<reference>();
			if(ref.is_null()) {
				thrown = create_null_pointer_exception();
				return handle_thrown();
			}
			stack.emplace_back(jint{ ::array_length(ref.object()) });
		}
		else if constexpr (same_as<Type, a_throw>) {
			if(info) { tabs(); fputs("a_throw\n", stderr); }

			reference ref = move(stack.pop_back().get<reference>());
			if(ref.is_null()) {
				ref = create_null_pointer_exception();
			}
			thrown = move(ref);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, check_cast>) {
			if(info) {
				tabs(); fputs("check_cast ", stderr);
				fprintf(stderr, "%hu\n", (uint16) x.index);
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
				fwrite(name.elements_ptr(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			reference objectref = stack.pop_back().get<reference>();
			_class& s = objectref.object()._class();

			int32 result = 0;
			if(!objectref.is_null()) {
				_class& t = c.get_class(x.index);

				if(!s.is_interface()) {
					if(!t.is_interface()) {
						result = &s == &t || s.is_sub_of(t);
					}
					else {
						result = s.is_implementing(t);
					}
				}
				else {
					abort();//TODO
				}
			}
			stack.emplace_back(jint{ result });
		}
		else if constexpr (same_as<Type, if_null>) {
			if(info) {
				tabs(); fputs("if_null ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			reference ref = stack.pop_back().get<reference>();
			if(ref.is_null()) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_non_null>) {
			if(info) {
				tabs(); fputs("if_non_null ", stderr);
				fprintf(stderr, "%hd\n", x.branch);
			}
			reference ref = stack.pop_back().get<reference>();
			if(!ref.is_null()) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, uint8>) {
			if(info) tabs();
			fprintf(stderr, "unknown instruction ");
			for_each_digit_in_number(
				x, number_base{ 10 },
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