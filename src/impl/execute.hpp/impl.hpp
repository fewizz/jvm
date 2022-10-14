#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/latest_context.hpp"
#include "decl/thrown.hpp"
#include "decl/array.hpp"
#include "decl/object/create.hpp"
#include "decl/native/interface/call.hpp"
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
#include "./select_method.hpp"
#include "./select_method_for_invoke_special.hpp"

#include <class_file/reader.hpp>
#include <class_file/descriptor/method_reader.hpp>
#include <class_file/attribute/code/reader.hpp>

#include <number.hpp>
#include <c_string.hpp>
#include <on_scope_exit.hpp>
#include <max.hpp>
#include <range.hpp>

#include <posix/math.hpp>

static void execute(method& m) {
	namespace cf = class_file;
	namespace instr = cf::attribute::code::instruction;

	_class& c = m._class();
	if(info) {
		tabs();
		print("executing: ");
		print(c.name());
		print(".");
		print(m.name());
		print(m.descriptor());
		print(" ");
		print("max_stack: ");
		print(m.code().max_stack);
		print("\n");
		++tab;
	}

	on_scope_exit bring_tab_back {
		[] { if(info) { --tab; } }
	};

	uint32 pc = 0;

	execution_context ctx {
		m,
		latest_execution_context
	};

	latest_execution_context = ctx;

	on_scope_exit set_latest_execution_context_to_previous {
		[&] { latest_execution_context = ctx.previous; }
	};

	if(m.is_native()) {
		if(!m.native_function_is_loaded()) {
			abort();
		}
		native_function_ptr ptr = m.native_function();
		native_interface_call(ptr, m);
		return;
	}

	if(m.code().iterator() == nullptr) {
		posix::std_err.write_from(c_string{ "no code\n" });
		abort();
	}

	nuint locals_begin = stack.size() - m.parameters_stack_size();
	nuint locals_end = locals_begin + m.code().max_locals;
	nuint stack_begin = locals_end;

	{
		nuint locals_pushed = m.parameters_stack_size();
		while(locals_pushed < m.code().max_locals) {
			stack.emplace_back(int32{ -1 });
			++locals_pushed;
		}
	}

	namespace attr = cf::attribute;
	using namespace attr::code::instruction;
	namespace cc = cf::constant;

	auto on_unimplemented_instruction = [] (uint8 code) {
		if(info) tabs();
		posix::std_err.write_from(c_string{ "unimplemented instruction " });
		for_each_digit_in_number(
			code, number_base{ 10 },
			[](auto digit) {
				posix::std_err.write_from(array{ '0' + digit });
			}
		);
		posix::std_err.write_from(c_string{ "\n" });
		abort();
	};

	cf::attribute::code::reader<
		uint8*,
		cf::attribute::code::reader_stage::code
	> reader{ m.code().iterator() };

	reader([&]<typename Type>(Type x, uint8*& it) {
		on_scope_exit update_pc{[&](){
			pc = it - m.code().iterator();
		}};

		auto handle_thrown = [&]() -> loop_action {
			if(thrown.is_null()) {
				return loop_action::next;
			}

			_class& thrown_class = thrown->_class();

			auto& exception_handlers = m.exception_handlers();

			for(attr::code::exception_handler handler : exception_handlers) {
				bool in_range = pc >= handler.start_pc && pc < handler.end_pc;
				if(!in_range) {
					continue;
				}

				_class& catch_class = c.get_resolved_class(handler.catch_type);

				bool same = &thrown_class == &catch_class;
				bool subclass = thrown_class.is_sub_of(catch_class);

				if(!(same || subclass)) {
					continue;
				}

				it = m.code().iterator() + handler.handler_pc;

				stack.pop_back_until(stack_begin);
				stack.emplace_back(move(thrown));
				return loop_action::next;
			}

			return loop_action::stop;
		};

		auto view_array = [&]<typename E, typename Handler>(
			Handler&& handler
		) {
			int32 element_index = stack.pop_back<int32>();
			reference array_ref = stack.pop_back<reference>();
			if(array_ref.is_null()) {
				thrown = create_null_pointer_exception();
				return handle_thrown();
			}
			int32 len = ::array_length(array_ref);
			if(element_index < 0 || element_index >= len) {
				thrown = create_index_of_of_bounds_exception();
				return handle_thrown();
			}
			E* ptr = array_data<E>(array_ref);
			handler(ptr[element_index]);
			return loop_action::next;
		};

		if constexpr (same_as<Type, nop>) {}
		else if constexpr (same_as<Type, a_const_null>) {
			if(info) { tabs(); print("a_const_null\n"); }
			stack.emplace_back(reference{});
		}
		else if constexpr (same_as<Type, i_const_m1>) {
			if(info) { tabs(); print("i_const_m1\n"); }
			stack.emplace_back(int32{ -1 });
		}
		else if constexpr (same_as<Type, i_const_0>) {
			if(info) { tabs(); print("i_const_0\n"); }
			stack.emplace_back(int32{ 0 });
		}
		else if constexpr (same_as<Type, i_const_1>) {
			if(info) { tabs(); print("i_const_1\n"); }
			stack.emplace_back(int32{ 1 });
		}
		else if constexpr (same_as<Type, i_const_2>) {
			if(info) { tabs(); print("i_const_2\n"); }
			stack.emplace_back(int32{ 2 });
		}
		else if constexpr (same_as<Type, i_const_3>) {
			if(info) { tabs(); print("i_const_3\n"); }
			stack.emplace_back(int32{ 3 });
		}
		else if constexpr (same_as<Type, i_const_4>) {
			if(info) { tabs(); print("i_const_4\n"); }
			stack.emplace_back(int32{ 4 });
		}
		else if constexpr (same_as<Type, i_const_5>) {
			if(info) { tabs(); print("i_const_5\n"); }
			stack.emplace_back(int32{ 5 });
		}
		else if constexpr (same_as<Type, l_const_0>) {
			if(info) { tabs(); print("l_const_0\n"); }
			stack.emplace_back(int64{ 0 });
		}
		else if constexpr (same_as<Type, l_const_1>) {
			if(info) { tabs(); print("l_const_1\n"); }
			stack.emplace_back(int64{ 1 });
		}
		else if constexpr (same_as<Type, f_const_0>) {
			if(info) { tabs(); print("f_const_0\n"); }
			stack.emplace_back(float{ 0.0F });
		}
		else if constexpr (same_as<Type, f_const_1>) {
			if(info) { tabs(); print("f_const_1\n"); }
			stack.emplace_back(float{ 1.0F });
		}
		else if constexpr (same_as<Type, f_const_2>) {
			if(info) { tabs(); print("f_const_2\n"); }
			stack.emplace_back(float{ 2.0F });
		}
		else if constexpr (same_as<Type, bi_push>) {
			if(info) {
				tabs();
				print("bi_push ");
				print(x.value);
				print("\n");
			}
			stack.emplace_back(int32{ x.value });
		}
		else if constexpr (same_as<Type, si_push>) {
			if(info) {
				tabs();
				print("si_push ");
				print(x.value);
				print("\n");
			}
			stack.emplace_back(int32{ x.value });
		}
		else if constexpr (
			same_as<Type, instr::ldc> || same_as<Type, instr::ldc_w>
		) {
			::ldc(x.index, c);
		}
		else if constexpr (same_as<Type, instr::ldc_2_w>) {
			::ldc_2_w(x.index, c);
		}

		else if constexpr (same_as<Type, i_load>) {
			if(info) {
				tabs();
				print("i_load ");
				print(x.index);
				print("\n");
			}
			stack.emplace_back(stack.at<int32>(locals_begin + x.index));
		}
		else if constexpr (same_as<Type, l_load>) {
			if(info) {
				tabs();
				print("l_load ");
				print(x.index);
				print("\n");
			}
			stack.emplace_back(stack.at<int64>(locals_begin + x.index));
		}
		else if constexpr (same_as<Type, f_load>) {
			if(info) {
				tabs();
				print("f_load ");
				print(x.index);
				print("\n");
			}
			stack.emplace_back(stack.at<float>(locals_begin + x.index));
		}
		else if constexpr (same_as<Type, a_load>) {
			if(info) {
				tabs();
				print("a_load ");
				print(x.index);
				print("\n");
			}
			stack.emplace_back(stack.at<reference>(locals_begin + x.index));
		}
		else if constexpr (same_as<Type, i_load_0>) {
			int32 value = stack.at<int32>(locals_begin + 0);
			if(info) {
				tabs();
				print("i_load_0 ");
				print(value);
				print("\n");
			}
			stack.emplace_back(value);
		}
		else if constexpr (same_as<Type, i_load_1>) {
			int32 value = stack.at<int32>(locals_begin + 1);
			if(info) {
				tabs();
				print("i_load_1 ");
				print(value);
				print("\n");
			}
			stack.emplace_back(value);
		}
		else if constexpr (same_as<Type, i_load_2>) {
			int32 value = stack.at<int32>(locals_begin + 2);
			if(info) {
				tabs();
				print("i_load_2 ");
				print(value);
				print("\n");
			}
			stack.emplace_back(value);
		}
		else if constexpr (same_as<Type, i_load_3>) {
			int32 value = stack.at<int32>(locals_begin + 3);
			if(info) {
				tabs();
				print("i_load_3 ");
				print(value);
				print("\n");
			}
			stack.emplace_back(value);
		}
		else if constexpr (same_as<Type, l_load_0>) {
			if(info) { tabs(); print("l_load_0\n"); }
			stack.emplace_back(stack.at<int64>(locals_begin + 0));
		}
		else if constexpr (same_as<Type, l_load_1>) {
			if(info) { tabs(); print("l_load_1\n"); }
			stack.emplace_back(stack.at<int64>(locals_begin + 1));
		}
		else if constexpr (same_as<Type, l_load_2>) {
			if(info) { tabs(); print("l_load_2\n"); }
			stack.emplace_back(stack.at<int64>(locals_begin + 2));
		}
		else if constexpr (same_as<Type, l_load_3>) {
			if(info) { tabs(); print("l_load_3\n"); }
			stack.emplace_back(stack.at<int64>(locals_begin + 3));
		}
		else if constexpr (same_as<Type, f_load_0>) {
			if(info) { tabs(); print("f_load_0\n"); }
			stack.emplace_back(stack.at<float>(locals_begin + 0));
		}
		else if constexpr (same_as<Type, f_load_1>) {
			if(info) { tabs(); print("f_load_1\n"); }
			stack.emplace_back(stack.at<float>(locals_begin + 1));
		}
		else if constexpr (same_as<Type, f_load_2>) {
			if(info) { tabs(); print("f_load_2\n"); }
			stack.emplace_back(stack.at<float>(locals_begin + 2));
		}
		else if constexpr (same_as<Type, f_load_3>) {
			if(info) { tabs(); print("f_load_3\n"); }
			stack.emplace_back(stack.at<float>(locals_begin + 3));
		}
		else if constexpr (same_as<Type, a_load_0>) {
			reference ref = stack.at<reference>(locals_begin + 0);
			if(info) {
				tabs(); print("a_load_0 ");
				if(!ref.is_null()) {
					print(ref._class().name());
				}
				print(c_string{" @ "});
				print((uint64) ref.object_ptr());
				print(c_string{ "\n" });
			}
			stack.emplace_back(move(ref));
		}
		else if constexpr (same_as<Type, a_load_1>) {
			if(info) { tabs(); print("a_load_1\n"); }
			reference ref = stack.at<reference>(locals_begin + 1);
			stack.emplace_back(move(ref));
		}
		else if constexpr (same_as<Type, a_load_2>) {
			reference ref = stack.at<reference>(locals_begin + 2);
			if(info) {
				tabs(); print("a_load_2 ");
				if(!ref.is_null()) {
					print(ref._class().name());
				}
				print(c_string{" @ "});
				print((uint64) ref.object_ptr());
				print(c_string{ "\n" });
			}
			stack.emplace_back(move(ref));
		}
		else if constexpr (same_as<Type, a_load_3>) {
			if(info) { tabs(); print("a_load_3\n"); }
			reference ref = stack.at<reference>(locals_begin + 3);
			stack.emplace_back(move(ref));
		}
		else if constexpr (same_as<Type, i_a_load>) {
			if(info) { tabs(); print("i_a_load\n"); }
			return view_array.template operator()<int32>([&](int32& v) {
				stack.emplace_back(int32{ v });
			});
		}
		else if constexpr (same_as<Type, a_a_load>) {
			if(info) { tabs(); print("a_a_load\n"); }
			return view_array.template operator()<reference>([&](reference& v) {
				stack.emplace_back(v);
			});
		}
		else if constexpr (same_as<Type, b_a_load>) {
			if(info) { tabs(); print("b_a_load\n"); }
			return view_array.template operator()<int8>([&](int8& v) {
				stack.emplace_back(int32{ uint8(v) });
			});
		}
		else if constexpr (same_as<Type, c_a_load>) {
			if(info) { tabs(); print("c_a_load\n"); }
			return view_array.template operator()<int16>([&](int16& v) {
				stack.emplace_back(int32{ uint16(v) });
			});
		}
		else if constexpr (same_as<Type, i_store>) {
			if(info) {
				tabs();
				print("i_store ");
				print(x.index);
				print("\n");
			}
			stack.at(locals_begin + x.index, stack.pop_back<int32>());
		}
		else if constexpr (same_as<Type, l_store>) {
			if(info) {
				tabs();
				print("l_store ");
				print(x.index);
				print("\n");
			}
			stack.at(locals_begin + x.index, stack.pop_back<int64>());
		}
		else if constexpr (same_as<Type, f_store>) {
			if(info) {
				tabs();
				print("f_store ");
				print(x.index);
				print("\n");
			}
			stack.at(locals_begin + x.index, stack.pop_back<float>());
		}
		else if constexpr (same_as<Type, a_store>) {
			if(info) {
				tabs();
				print("a_store ");
				print(x.index);
				print("\n");
			}
			stack.emplace_at(
				locals_begin + x.index, stack.pop_back<reference>()
			);
		}
		else if constexpr (same_as<Type, i_store_0>) {
			if(info) { tabs(); print("i_store_0\n"); }
			stack.at(locals_begin + 0, stack.pop_back<int32>());
		}
		else if constexpr (same_as<Type, i_store_1>) {
			if(info) { tabs(); print("i_store_1\n"); }
			stack.at(locals_begin + 1, stack.pop_back<int32>());
		}
		else if constexpr (same_as<Type, i_store_2>) {
			if(info) { tabs(); print("i_store_2\n"); }
			stack.at(locals_begin + 2, stack.pop_back<int32>());
		}
		else if constexpr (same_as<Type, i_store_3>) {
			if(info) { tabs(); print("i_store_3\n"); }
			stack.at(locals_begin + 3, stack.pop_back<int32>());
		}
		else if constexpr (same_as<Type, l_store_0>) {
			if(info) { tabs(); print("l_store_0\n"); }
			stack.at(locals_begin + 0, stack.pop_back<int64>());
		}
		else if constexpr (same_as<Type, l_store_1>) {
			if(info) { tabs(); print("l_store_1\n"); }
			stack.at(locals_begin + 1, stack.pop_back<int64>());
		}
		else if constexpr (same_as<Type, l_store_2>) {
			if(info) { tabs(); print("l_store_2\n"); }
			stack.at(locals_begin + 2, stack.pop_back<int64>());
		}
		else if constexpr (same_as<Type, l_store_3>) {
			if(info) { tabs(); print("l_store_3\n"); }
			stack.at(locals_begin + 3, stack.pop_back<int64>());
		}
		else if constexpr (same_as<Type, a_store_0>) {
			if(info) { tabs(); print("a_store_0\n"); }
			stack.emplace_at(locals_begin + 0, stack.pop_back<reference>());
		}
		else if constexpr (same_as<Type, a_store_1>) {
			if(info) { tabs(); print("a_store_1\n"); }
			stack.emplace_at(locals_begin + 1, stack.pop_back<reference>());
		}
		else if constexpr (same_as<Type, a_store_2>) {
			if(info) { tabs(); print("a_store_2\n"); }
			stack.emplace_at(locals_begin + 2, stack.pop_back<reference>());
		}
		else if constexpr (same_as<Type, a_store_3>) {
			if(info) { tabs(); print("a_store_3\n"); }
			stack.emplace_at(locals_begin + 3, stack.pop_back<reference>());
		}
		else if constexpr (same_as<Type, i_a_store>) {
			if(info) { tabs(); print("i_a_store\n"); }
			int32 value = stack.pop_back<int32>();
			return view_array.template operator()<int32>([&](int32& v) {
				v = value;
			});
		}
		else if constexpr (same_as<Type, a_a_store>) {
			if(info) { tabs(); print("a_a_store\n"); }
			reference value = stack.pop_back<reference>();
			return view_array.template operator()<reference>([&](reference& v) {
				v = value;
			});
		}
		else if constexpr (same_as<Type, b_a_store>) {
			if(info) { tabs(); print("b_a_store\n"); }
			int32 value = stack.pop_back<int32>();
			return view_array.template operator()<int8>([&](int8& v) {
				v = (uint8) uint32(value);
			});
		}
		else if constexpr (same_as<Type, c_a_store>) {
			if(info) { tabs(); print("c_a_store\n"); }
			int32 value = stack.pop_back<int32>();
			return view_array.template operator() <int16>([&](int16& v) {
				v = (uint16) uint32(value);
			});
		}

		else if constexpr (same_as<Type, pop>) {
			if(info) { tabs(); print("pop\n"); }
			stack.pop_back();
		}
		else if constexpr (same_as<Type, dup>) {
			if(info) { tabs(); print("dup\n"); }
			stack.dup_cat_1();
		}
		else if constexpr (same_as<Type, dup_x1>) {
			if(info) { tabs(); print("dup_x1\n"); }
			stack.dup_x1_cat_1();
		}
		else if constexpr (same_as<Type, dup_2>) {
			if(info) { tabs(); print("dup_2\n"); }
			stack.dup2();
		}

		else if constexpr (same_as<Type, i_add>) {
			if(info) { tabs(); print("i_add\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 + value2 });
		}
		else if constexpr (same_as<Type, l_add>) {
			if(info) { tabs(); print("l_add\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 + value2 });
		}
		else if constexpr (same_as<Type, f_add>) {
			if(info) { tabs(); print("f_add\n"); }
			float value2 = stack.pop_back<float>();
			float value1 = stack.pop_back<float>();
			stack.emplace_back(float{ value1 + value2 });
		}
		else if constexpr (same_as<Type, i_sub>) {
			if(info) { tabs(); print("i_sub\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 - value2 });
		}
		else if constexpr (same_as<Type, l_sub>) {
			if(info) { tabs(); print("l_sub\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 - value2 });
		}
		else if constexpr (same_as<Type, i_mul>) {
			if(info) { tabs(); print("i_mul\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 * value2 });
		}
		else if constexpr (same_as<Type, l_mul>) {
			if(info) { tabs(); print("l_mul\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 * value2 });
		}
		else if constexpr (same_as<Type, f_mul>) {
			if(info) { tabs(); print("f_mul\n"); }
			float value2 = stack.pop_back<float>();
			float value1 = stack.pop_back<float>();
			stack.emplace_back(float{ value1 * value2 });
		}
		else if constexpr (same_as<Type, i_div>) {
			if(info) { tabs(); print("i_div\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 / value2 });
		}
		else if constexpr (same_as<Type, f_div>) {
			if(info) { tabs(); print("f_div\n"); }
			float value2 = stack.pop_back<float>();
			float value1 = stack.pop_back<float>();
			stack.emplace_back(float{ value1 / value2 });
		}
		else if constexpr (same_as<Type, i_rem>) {
			if(info) { tabs(); print("i_rem\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 - (value1 / value2) * value2 });
		}
		else if constexpr (same_as<Type, i_neg>) {
			if(info) { tabs(); print("i_neg\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back(int32{ -value });
		}
		else if constexpr (same_as<Type, i_sh_l>) {
			if(info) { tabs(); print("i_sh_l\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 << (value2 & 0x1F) });
		}
		else if constexpr (same_as<Type, l_sh_l>) {
			if(info) { tabs(); print("l_sh_l\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 << (value2 & 0x3F) });
		}
		else if constexpr (same_as<Type, i_sh_r>) {
			if(info) { tabs(); print("i_sh_r\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 >> (value2 & 0x1F) });
		}
		else if constexpr (same_as<Type, l_sh_r>) {
			if(info) { tabs(); print("l_sh_r\n"); }
			int32 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 >> (value2 & 0x3F) });
		}
		else if constexpr (same_as<Type, i_u_sh_r>) {
			if(info) { tabs(); print("i_u_sh_r\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32 {
				int32(uint32(value1) >> (value2 & 0x1F))
			});
		}
		else if constexpr (same_as<Type, l_u_sh_r>) {
			if(info) { tabs(); print("l_ush_r\n"); }
			int32 value2 = stack.pop_back<int32>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64 {
				int64(uint64(value1) >> (value2 & 0x3F))
			});
		}
		else if constexpr (same_as<Type, i_and>) {
			if(info) { tabs(); print("i_and\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 & value2 });
		}
		else if constexpr (same_as<Type, l_and>) {
			if(info) { tabs(); print("l_and\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 & value2 });
		}
		else if constexpr (same_as<Type, i_or>) {
			if(info) { tabs(); print("i_or\n"); }
			int32 value2 = stack.pop_back<int64>();
			int32 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 | value2 });
		}
		else if constexpr (same_as<Type, l_or>) {
			if(info) { tabs(); print("l_or\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 | value2 });
		}
		else if constexpr (same_as<Type, i_xor>) {
			if(info) { tabs(); print("i_xor\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 ^ value2 });
		}
		else if constexpr (same_as<Type, i_inc>) {
			if(info) {
				tabs();
				print("i_inc ");
				print(x.index);
				print(" ");
				print(x.value);
				print("\n");
			}
			stack.at<int32>(locals_begin + x.index) += x.value;
		}
		else if constexpr (same_as<Type, i_to_l>) {
			if(info) { tabs(); print("i_to_l\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back((int64) value);
		}
		else if constexpr (same_as<Type, i_to_f>) {
			if(info) { tabs(); print("i_to_f\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back((float) value);
		}
		else if constexpr (same_as<Type, l_to_i>) {
			if(info) { tabs(); print("l_to_i\n"); }
			int64 value = stack.pop_back<int64>();
			stack.emplace_back((int32) (value & 0xFFFFFFFF));
		}
		else if constexpr (same_as<Type, f_to_i>) {
			if(info) { tabs(); print("f_to_i\n"); }
			float value = stack.pop_back<float>();
			int32 result;
			if(posix::is_nan(value)) {
				result = 0;
			}
			else {
				result = (int32) value;
			}
			stack.emplace_back(result);
		}
		else if constexpr (same_as<Type, i_to_b>) {
			if(info) { tabs(); print("i_to_b\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back((int32) (int8) value);
		}
		else if constexpr (same_as<Type, i_to_c>) {
			if(info) { tabs(); print("i_to_c\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back((int32) (int16) value);
		}

		else if constexpr (same_as<Type, l_cmp>) {
			if(info) { tabs(); print("l_cmp\n"); }
			int64 value_2 = stack.pop_back<int64>();
			int64 value_1 = stack.pop_back<int64>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			if(value_1 == value_2) result =  0;
			if(value_1 <  value_2) result = -1;
			stack.emplace_back(result);
		}
		else if constexpr (same_as<Type, f_cmp_l>) {
			if(info) { tabs(); print("f_cmp_l\n"); }
			float value_2 = stack.pop_back<float>();
			float value_1 = stack.pop_back<float>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			else if(value_1 == value_2) result =  0;
			else if(value_1 <  value_2) result = -1;
			else { // NaN
				result = -1;
			}
			stack.emplace_back(result);
		}
		else if constexpr (same_as<Type, f_cmp_g>) {
			if(info) { tabs(); print("f_cmp_g\n"); }
			float value_2 = stack.pop_back<float>();
			float value_1 = stack.pop_back<float>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			else if(value_1 == value_2) result =  0;
			else if(value_1 <  value_2) result = -1;
			else { // NaN
				result = 1;
			}
			stack.emplace_back(result);
		}
		else if constexpr (same_as<Type, if_eq>) {
			if(info) {
				tabs();
				print("if_eq ");
				print(x.branch);
				print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value == 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_ne>) {
			if(info) {
				tabs();
				print("if_ne ");
				print(x.branch);
				print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value != 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_lt>) {
			if(info) {
				tabs();
				print("if_lt ");
				print(x.branch);
				print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value < 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_ge>) {
			if(info) {
				tabs();
				print("if_ge ");
				print(x.branch);
				print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value >= 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_gt>) {
			if(info) {
				tabs();
				print("if_gt ");
				print(x.branch);
				print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value > 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_le>) {
			if(info) {
				tabs();
				print("if_le ");
				print(x.branch);
				print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value <= 0) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_eq>) {
			if(info) {
				tabs();
				print("if_i_cmp_eq ");
				print(x.branch);
				print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 == value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_ne>) {
			if(info) {
				tabs();
				print("if_i_cmp_ne ");
				print(x.branch);
				print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 != value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_lt>) {
			if(info) {
				tabs();
				print("if_i_cmp_lt ");
				print(x.branch);
				print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 < value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_ge>) {
			if(info) {
				tabs();
				print("if_i_cmp_ge ");
				print(x.branch);
				print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 >= value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_gt>) {
			if(info) {
				tabs();
				print("if_i_cmp_gt ");
				print(x.branch);
				print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 > value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_le>) {
			if(info) {
				tabs();
				print("if_i_cmp_le ");
				print(x.branch);
				print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 <= value2) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_a_cmp_eq>) {
			if(info) {
				tabs();
				print("if_a_cmp_eq ");
				print(x.branch);
				print("\n");
			}
			reference value2 = stack.pop_back<reference>();
			reference value1 = stack.pop_back<reference>();
			if(value1.object_ptr() == value2.object_ptr()) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_a_cmp_ne>) {
			if(info) {
				tabs();
				print("if_a_cmp_ne ");
				print(x.branch);
				print("\n");
			}
			reference value2 = stack.pop_back<reference>();
			reference value1 = stack.pop_back<reference>();
			if(value1.object_ptr() != value2.object_ptr()) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, go_to>) {
			if(info) {
				tabs();
				print("go_to ");
				print(x.branch);
				print("\n");
			}
			it = m.code().iterator() + pc + x.branch;
		}
		else if constexpr (same_as<Type, i_return>) {
			if(info) { tabs(); print("i_return\n"); }
			int32 result = stack.back<int32>();
			m.return_type().view([&]<typename RetType>(RetType) {
				if constexpr(same_as<RetType, class_file::z>) {
					result = result == 1;
				}
				if constexpr(same_as<RetType, class_file::b>) {
					result = (uint32) (int8) result;
				}
				if constexpr(same_as<RetType, class_file::c>) {
					result = (uint32) (uint16) result;
				}
				if constexpr(same_as<RetType, class_file::s>) {
					result = (uint32) ( int16) result;
				}
			});
			stack.pop_back_until(locals_begin);
			stack.emplace_back(result);
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, l_return>) {
			if(info) { tabs(); print("l_return\n"); }
			int64 result = stack.pop_back<int64>();
			stack.pop_back_until(locals_begin);
			stack.emplace_back(result);
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, f_return>) {
			if(info) { tabs(); print("f_return\n"); }
			float result = stack.pop_back<float>();
			stack.pop_back_until(locals_begin);
			stack.emplace_back(result);
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, d_return>) {
			if(info) { tabs(); print("d_return\n"); }
			double result = stack.pop_back<double>();
			stack.pop_back_until(locals_begin);
			stack.emplace_back(result);
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, a_return>) {
			if(info) { tabs(); print("a_return\n"); }
			reference result = stack.pop_back<reference>();
			stack.pop_back_until(locals_begin);
			stack.emplace_back(move(result));
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, _return>) {
			if(info) { tabs(); print("return\n"); }
			stack.pop_back_until(locals_begin);
			return loop_action::stop;
		}
		else if constexpr (same_as<Type, get_static>) {
			if(info) {
				tabs(); print("get_static ");
				cc::field_ref field_ref = c.field_ref_constant(x.index);
				cc::_class _class = c.class_constant(field_ref.class_index);
				cc::utf8 class_name = c.utf8_constant(_class.name_index);
				cc::name_and_type nat {
					c.name_and_type_constant(field_ref.name_and_type_index)
				};
				cc::utf8 name = c.utf8_constant(nat.name_index);
				print(class_name);
				print(".");
				print(name);
				print("\n");
			}
			field_value& value = c.get_static_field_value(x.index);
			get_field_value(value);
		}
		else if constexpr (same_as<Type, put_static>) {
			if(info) {
				tabs(); print("put_static ");
				cc::field_ref field_ref = c.field_ref_constant(x.index);
				cc::name_and_type nat {
					c.name_and_type_constant(field_ref.name_and_type_index)
				};
				cc::utf8 name = c.utf8_constant(nat.name_index);
				print(name);
				print("\n");
			}
			field_value& field_value = c.get_static_field_value(x.index);
			put_field_value(field_value);
		}
		else if constexpr (same_as<Type, get_field>) {
			if(info) {
				tabs(); print("get_field ");
				auto field_ref = c.field_ref_constant(x.index);
				cc::name_and_type nat = c.name_and_type_constant(
					field_ref.name_and_type_index
				);
				cc::_class class_ = c.class_constant(field_ref.class_index);
				cc::utf8 class_name = c.utf8_constant(class_.name_index);
				cc::utf8 field_name = c.utf8_constant(nat.name_index);
				print(class_name);
				print(".");
				print(field_name);
				print("\n");
			}

			tuple<instance_field_index, _class&> index_and_class {
				c.get_resolved_instance_field_index_and_class(x.index)
			};

			reference ref = stack.pop_back<reference>();
			if(ref.is_null()) {
				thrown = create_null_pointer_exception();
				return handle_thrown();
			}
			field_value& value = ref[
				index_and_class.get<instance_field_index>()
			];
			get_field_value(value);
		}
		else if constexpr (same_as<Type, put_field>) {
			if(info) {
				tabs(); print("put_field ");
				cc::field_ref field_ref = c.field_ref_constant(x.index);
				cc::name_and_type nat = c.name_and_type_constant(
					field_ref.name_and_type_index
				);
				cc::_class class_ = c.class_constant(field_ref.class_index);
				cc::utf8 class_name = c.utf8_constant(class_.name_index);
				cc::utf8 name = c.utf8_constant(nat.name_index);
				print(class_name);
				print(".");
				print(name);
				print("\n");
			}

			tuple<instance_field_index, _class&> index_and_class {
				c.get_resolved_instance_field_index_and_class(x.index)
			};
			instance_field_index index =
				index_and_class.get<instance_field_index>();
			_class& base_c = index_and_class.get<_class&>();
			field* base_field = base_c.instance_fields()[index];
			reference ref = stack.at<reference>(
				stack.size() - base_field->stack_size - 1
			);
			if(ref.is_null()) {
				thrown = create_null_pointer_exception();
				return handle_thrown();
			}

			field_value& to = ref[index];
			put_field_value(to);
			stack.pop_back(); // pop reference to this
		}
		else if constexpr (same_as<Type, instr::invoke_virtual>) {
			::invoke_virtual(x.index, c);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_special>) {
			::invoke_special(x.index, c);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_static>) {
			::invoke_static(x.index, c);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_interface>) {
			::invoke_interface(x.index, c);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, instr::invoke_dynamic>) {
			::invoke_dynamic(x.index, c);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, _new>) {
			if(info) {
				tabs(); print("new ");
				cc::utf8 name = c.utf8_constant(
					c.class_constant(x.index).name_index
				);
				print(name);
				print("\n");
			}
			_class& c0 = c.get_resolved_class(x.index);
			stack.emplace_back(create_object(c0));
		}
		else if constexpr (same_as<Type, instr::new_array>) {
			::new_array(/* c, */ x.type);
		}
		else if constexpr (same_as<Type, a_new_array>) {
			_class& element_class = c.get_resolved_class(x.index);

			if(info) {
				tabs(); print("a_new_array ");
				cc::utf8 name = element_class.name();
				print(name);
				print("\n");
			}

			int32 count = stack.pop_back<int32>();
			auto ref = create_array_of(element_class, count);
			stack.emplace_back(move(ref));
		}
		else if constexpr (same_as<Type, instr::array_length>) {
			if(info) { tabs(); print("array_length\n"); }
			reference ref = stack.pop_back<reference>();
			if(ref.is_null()) {
				thrown = create_null_pointer_exception();
				return handle_thrown();
			}
			stack.emplace_back(int32{ ::array_length(ref) });
		}
		else if constexpr (same_as<Type, a_throw>) {
			if(info) { tabs(); print("a_throw\n"); }

			reference ref = move(stack.pop_back<reference>());
			if(ref.is_null()) {
				ref = create_null_pointer_exception();
			}
			thrown = move(ref);
			return handle_thrown();
		}
		else if constexpr (same_as<Type, check_cast>) {
			if(info) {
				tabs();
				print("check_cast ");
				print((uint16) x.index);
				print("\n");
			}
			reference& ref = stack.back<reference>();
			if(ref.is_null()) {
				return loop_action::next;
			}

			_class& s = ref._class();
			_class& t = c.get_resolved_class(x.index);
			
			bool result;

			if(!s.is_array()) {
				if(!t.is_interface()) {
					result = &s == &t || s.is_sub_of(t);
				} else {
					result = s.is_implementing(t);
				}
			} else {
				if(!t.is_interface() && !t.is_array()) {
					result = &t == object_class.ptr();
				}
				else if(t.is_interface()) {
					abort(); // unimplemented
				}
				else {
					_class& sc = s.get_component_class();
					_class& tc = t.get_component_class();
					if(sc.is_primitive() && tc.is_primitive()) {
						result = &sc == &tc;
					}
					else if(!sc.is_primitive() && !tc.is_primitive()) {
						abort(); // unimplemented
					}
					else {
						result = false;
					}
				}
				
			}

			if(!result) {
				abort(); // TODO throw ClassCastException
			}
			//_class& type = c.get_resolved_class(x.index);
			//reference objectref = stack[stack_size - 1].get<reference>();
			// TODO
		}
		else if constexpr (same_as<Type, instance_of>) {
			if(info) {
				cc::_class _class = c.class_constant(x.index);
				cc::utf8 name = c.utf8_constant(_class.name_index);
				tabs();
				print("instance_of ");
				print(name);
				print("\n");
			}

			reference objectref = stack.pop_back<reference>();
			_class& s = objectref->_class();

			int32 result = 0;
			if(!objectref.is_null()) {
				_class& t = c.get_resolved_class(x.index);

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
			stack.emplace_back(int32{ result });
		}
		else if constexpr (same_as<Type, if_null>) {
			if(info) {
				tabs();
				print("if_null ");
				print(x.branch);
				print("\n");
			}
			reference ref = stack.pop_back<reference>();
			if(ref.is_null()) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, if_non_null>) {
			if(info) {
				tabs();
				print("if_non_null ");
				print(x.branch);
				print("\n");
			}
			reference ref = stack.pop_back<reference>();
			if(!ref.is_null()) {
				it = m.code().iterator() + pc + x.branch;
			}
		}
		else if constexpr (same_as<Type, uint8>) {
			if(info) tabs();
			print("unknown instruction ");
			print(x);
			abort();
		}
		else {
			on_unimplemented_instruction(Type::code);
		}

		return loop_action::next;

	}, m.code().size());
}