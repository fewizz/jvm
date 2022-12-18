#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/latest_context.hpp"
#include "decl/thrown.hpp"
#include "decl/array.hpp"
#include "decl/native/call.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/lib/java/lang/index_out_of_bounds_exception.hpp"
#include "decl/lib/java/lang/class_cast_exception.hpp"
#include "decl/lib/java/lang/stack_overflow_error.hpp"

#include "./check_cast.hpp"
#include "./instance_of.hpp"
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
#include <overloaded.hpp>

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
		++tab;
	}

	on_scope_exit bring_tab_back {
		[] { if(info) { --tab; } }
	};

	execution_context ctx {
		m,
		latest_execution_context
	};

	latest_execution_context = ctx;

	on_scope_exit set_latest_execution_context_to_previous {
		[&] { latest_execution_context = ctx.previous; }
	};

	if(m.is_native()) {
		if(info) {
			print("\n");
		}
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

	if(info) {
		print(" ");
		print("max_stack: "); print(m.code().max_stack);
		print(" locals begin: "); print(locals_begin);
		print(" stack begin: "); print(stack_begin);
		print("\n");
	}

	{
		nuint max_possible_stack_end = stack_begin + m.code().max_stack * 2;
		if(max_possible_stack_end > stack.capacity()) {
			stack.pop_back_until(locals_begin);
			thrown = create_stack_overflow_error();
			return;
		}
	}

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
		number{ code }.for_each_digit(
			number_base{ 10 },
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
	> code_reader{ m.code().iterator() - sizeof(uint32) }; // TODO, messy

	uint8* instrution_ptr = m.code().iterator();

	code_reader.read_and_get_exception_table_reader(
	[&]<typename Type>(Type x0, uint8*& next_instruction_ptr) {
		on_scope_exit update_instruction_ptr{[&] {
			instrution_ptr = next_instruction_ptr;
		}};

		auto handle_thrown = [&]() -> loop_action {
			if(thrown.is_null()) {
				return loop_action::next;
			}

			_class& thrown_class = thrown->_class();

			auto& exception_handlers = m.exception_handlers();

			for(attr::code::exception_handler handler : exception_handlers) {
				uint32 pc = instrution_ptr - m.code().iterator();
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

				next_instruction_ptr = m.code().iterator() + handler.handler_pc;

				stack.pop_back_until(stack_begin);
				stack.emplace_back(move(thrown));
				return loop_action::next;
			}
			stack.pop_back_until(locals_begin);
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

		return overloaded {
		[](nop) {},
		[](a_const_null) {
			if(info) { tabs(); print("a_const_null\n"); }
			stack.emplace_back(reference{});
		},
		[](i_const_m1) {
			if(info) { tabs(); print("i_const_m1\n"); }
			stack.emplace_back(int32{ -1 });
		},
		[](i_const_0) {
			if(info) { tabs(); print("i_const_0\n"); }
			stack.emplace_back(int32{ 0 });
		},
		[](i_const_1) {
			if(info) { tabs(); print("i_const_1\n"); }
			stack.emplace_back(int32{ 1 });
		},
		[](i_const_2) {
			if(info) { tabs(); print("i_const_2\n"); }
			stack.emplace_back(int32{ 2 });
		},
		[](i_const_3) {
			if(info) { tabs(); print("i_const_3\n"); }
			stack.emplace_back(int32{ 3 });
		},
		[](i_const_4) {
			if(info) { tabs(); print("i_const_4\n"); }
			stack.emplace_back(int32{ 4 });
		},
		[](i_const_5) {
			if(info) {
				tabs(); print("i_const_5 @"); print(stack.size()); print("\n");
			}
			stack.emplace_back(int32{ 5 });
		},
		[](l_const_0) {
			if(info) {
				tabs(); print("l_const_0 @"); print(stack.size()); print("\n");
			}
			stack.emplace_back(int64{ 0 });
		},
		[](l_const_1) {
			if(info) { tabs(); print("l_const_1\n"); }
			stack.emplace_back(int64{ 1 });
		},
		[](f_const_0) {
			if(info) { tabs(); print("f_const_0\n"); }
			stack.emplace_back(float{ 0.0F });
		},
		[](f_const_1) {
			if(info) { tabs(); print("f_const_1\n"); }
			stack.emplace_back(float{ 1.0F });
		},
		[](f_const_2) {
			if(info) { tabs(); print("f_const_2\n"); }
			stack.emplace_back(float{ 2.0F });
		},
		[](d_const_0) {
			if(info) { tabs(); print("d_const_0\n"); }
			stack.emplace_back(double{ 0.0 });
		},
		[](d_const_1) {
			if(info) { tabs(); print("d_const_1\n"); }
			stack.emplace_back(double{ 1.0 });
		},
		[](bi_push x) {
			if(info) {
				tabs();
				print("bi_push ");
				print(x.value);
				print(" @");
				print(stack.size());
				print("\n");
			}
			stack.emplace_back(int32{ x.value });
		},
		[](si_push x) {
			if(info) {
				tabs();
				print("si_push ");
				print(x.value);
				print("\n");
			}
			stack.emplace_back(int32{ x.value });
		},
		[&](same_as_any<instr::ldc, instr::ldc_w> auto x) {
			::ldc(x.index, c);
		},
		[&](instr::ldc_2_w x) {
			::ldc_2_w(x.index, c);
		},
		[&](i_load x) {
			int32 value = stack.get<int32>(locals_begin + x.index);
			if(info) {
				tabs();
				print("i_load ");
				print(x.index);
				print(" ");
				print(value);
				print("\n");
			}
			stack.emplace_back(value);
		},
		[&](l_load x) {
			if(info) {
				tabs();
				print("l_load ");
				print(x.index);
				print("\n");
			}
			stack.emplace_back(stack.get<int64>(locals_begin + x.index));
		},
		[&](f_load x) {
			if(info) {
				tabs();
				print("f_load ");
				print(x.index);
				print("\n");
			}
			stack.emplace_back(stack.get<float>(locals_begin + x.index));
		},
		[&](d_load x) {
			if(info) {
				tabs();
				print("d_load ");
				print(x.index);
				print("\n");
			}
			stack.emplace_back(stack.get<double>(locals_begin + x.index));
		},
		[&](a_load x) {
			if(info) {
				tabs();
				print("a_load ");
				print(x.index);
				print("\n");
			}
			stack.emplace_back(stack.get<reference>(locals_begin + x.index));
		},
		[&](i_load_0) {
			int32 value = stack.get<int32>(locals_begin + 0);
			if(info) {
				tabs();
				print("i_load_0 ");
				print(value);
				print("\n");
			}
			stack.emplace_back(value);
		},
		[&](i_load_1) {
			int32 value = stack.get<int32>(locals_begin + 1);
			if(info) {
				tabs();
				print("i_load_1 ");
				print(value);
				print("\n");
			}
			stack.emplace_back(value);
		},
		[&](i_load_2) {
			int32 value = stack.get<int32>(locals_begin + 2);
			if(info) {
				tabs();
				print("i_load_2 ");
				print(value);
				print("\n");
			}
			stack.emplace_back(value);
		},
		[&](i_load_3) {
			int32 value = stack.get<int32>(locals_begin + 3);
			if(info) {
				tabs();
				print("i_load_3 ");
				print(value);
				print("\n");
			}
			stack.emplace_back(value);
		},
		[&](l_load_0) {
			if(info) { tabs(); print("l_load_0\n"); }
			stack.emplace_back(stack.get<int64>(locals_begin + 0));
		},
		[&](l_load_1) {
			if(info) { tabs(); print("l_load_1\n"); }
			stack.emplace_back(stack.get<int64>(locals_begin + 1));
		},
		[&](l_load_2) {
			if(info) { tabs(); print("l_load_2\n"); }
			stack.emplace_back(stack.get<int64>(locals_begin + 2));
		},
		[&](l_load_3) {
			if(info) { tabs(); print("l_load_3\n"); }
			stack.emplace_back(stack.get<int64>(locals_begin + 3));
		},
		[&](f_load_0) {
			if(info) { tabs(); print("f_load_0\n"); }
			stack.emplace_back(stack.get<float>(locals_begin + 0));
		},
		[&](f_load_1) {
			if(info) { tabs(); print("f_load_1\n"); }
			stack.emplace_back(stack.get<float>(locals_begin + 1));
		},
		[&](f_load_2) {
			if(info) { tabs(); print("f_load_2\n"); }
			stack.emplace_back(stack.get<float>(locals_begin + 2));
		},
		[&](f_load_3) {
			if(info) { tabs(); print("f_load_3\n"); }
			stack.emplace_back(stack.get<float>(locals_begin + 3));
		},
		[&](d_load_0) {
			if(info) { tabs(); print("d_load_0\n"); }
			stack.emplace_back(stack.get<double>(locals_begin + 0));
		},
		[&](d_load_1) {
			if(info) { tabs(); print("d_load_1\n"); }
			stack.emplace_back(stack.get<double>(locals_begin + 1));
		},
		[&](d_load_2) {
			if(info) { tabs(); print("d_load_2\n"); }
			stack.emplace_back(stack.get<double>(locals_begin + 2));
		},
		[&](d_load_3) {
			if(info) { tabs(); print("d_load_3\n"); }
			stack.emplace_back(stack.get<double>(locals_begin + 3));
		},
		[&](a_load_0) {
			reference ref = stack.get<reference>(locals_begin + 0);
			if(info) {
				tabs(); print("a_load_0 ");
				if(!ref.is_null()) {
					print(ref._class().name());
				}
				print(c_string{" @"});
				print_hex((uint64) ref.object_ptr());
				print(c_string{ "\n" });
			}
			stack.emplace_back(move(ref));
		},
		[&](a_load_1) {
			if(info) { tabs(); print("a_load_1\n"); }
			reference ref = stack.get<reference>(locals_begin + 1);
			stack.emplace_back(move(ref));
		},
		[&](a_load_2) {
			reference ref = stack.get<reference>(locals_begin + 2);
			if(info) {
				tabs(); print("a_load_2 ");
				if(!ref.is_null()) {
					print(ref._class().name());
				}
				print(c_string{" @"});
				print_hex((uint64) ref.object_ptr());
				print(c_string{ "\n" });
			}
			stack.emplace_back(move(ref));
		},
		[&](a_load_3) {
			if(info) { tabs(); print("a_load_3\n"); }
			reference ref = stack.get<reference>(locals_begin + 3);
			stack.emplace_back(move(ref));
		},
		[&](i_a_load) {
			if(info) { tabs(); print("i_a_load\n"); }
			return view_array.template operator()<int32>([&](int32& v) {
				stack.emplace_back(v);
			});
		},
		[&](l_a_load) {
			if(info) { tabs(); print("l_a_load\n"); }
			return view_array.template operator()<int64>([&](int64& v) {
				stack.emplace_back(v);
			});
		},
		[&](f_a_load) {
			if(info) { tabs(); print("f_a_load\n"); }
			return view_array.template operator()<float>([&](float& v) {
				stack.emplace_back(v);
			});
		},
		[&](d_a_load) {
			if(info) { tabs(); print("d_a_load\n"); }
			return view_array.template operator()<double>([&](double& v) {
				stack.emplace_back(v);
			});
		},
		[&](a_a_load) {
			if(info) { tabs(); print("a_a_load\n"); }
			return view_array.template operator()<reference>([&](reference& v) {
				stack.emplace_back(v);
			});
		},
		[&](b_a_load) {
			if(info) { tabs(); print("b_a_load\n"); }
			return view_array.template operator()<int8>([&](int8& v) {
				stack.emplace_back(v);
			});
		},
		[&](c_a_load) {
			if(info) { tabs(); print("c_a_load\n"); }
			return view_array.template operator()<uint16>([&](uint16& v) {
				stack.emplace_back(v);
			});
		},
		[&](s_a_load) {
			if(info) { tabs(); print("s_a_load\n"); }
			return view_array.template operator()<int16>([&](int16& v) {
				stack.emplace_back(v);
			});
		},
		[&](i_store x) {
			int32 value = stack.pop_back<int32>();
			if(info) {
				tabs();
				print("i_store "); print(x.index); print(" "); print(value);
				print("\n");
			}
			stack.emplace_at(locals_begin + x.index, value);
		},
		[&](l_store x) {
			if(info) {
				tabs();
				print("l_store ");
				print(x.index);
				print("\n");
			}
			stack.emplace_at(locals_begin + x.index, stack.pop_back<int64>());
		},
		[&](f_store x) {
			if(info) {
				tabs();
				print("f_store ");
				print(x.index);
				print("\n");
			}
			stack.emplace_at(locals_begin + x.index, stack.pop_back<float>());
		},
		[&](d_store x) {
			if(info) {
				tabs();
				print("d_store ");
				print(x.index);
				print("\n");
			}
			stack.emplace_at(locals_begin + x.index, stack.pop_back<double>());
		},
		[&](a_store x) {
			if(info) {
				tabs();
				print("a_store ");
				print(x.index);
				print("\n");
			}
			stack.emplace_at(
				locals_begin + x.index, stack.pop_back<reference>()
			);
		},
		[&](i_store_0) {
			int32 value = stack.pop_back<int32>();
			if(info) { tabs();print("i_store_0 ");print(value);print("\n"); }
			stack.emplace_at(locals_begin + 0, value);
		},
		[&](i_store_1) {
			int32 value = stack.pop_back<int32>();
			if(info) { tabs();print("i_store_1 ");print(value);print("\n"); }
			stack.emplace_at(locals_begin + 1, value);
		},
		[&](i_store_2) {
			int32 value = stack.pop_back<int32>();
			if(info) { tabs();print("i_store_2 ");print(value);print("\n"); }
			stack.emplace_at(locals_begin + 2, value);
		},
		[&](i_store_3) {
			int32 value = stack.pop_back<int32>();
			if(info) { tabs();print("i_store_3 ");print(value);print("\n"); }
			stack.emplace_at(locals_begin + 3, value);
		},
		[&](l_store_0) {
			if(info) { tabs(); print("l_store_0\n"); }
			stack.emplace_at(locals_begin + 0, stack.pop_back<int64>());
		},
		[&](l_store_1) {
			if(info) { tabs(); print("l_store_1\n"); }
			stack.emplace_at(locals_begin + 1, stack.pop_back<int64>());
		},
		[&](l_store_2) {
			if(info) { tabs(); print("l_store_2\n"); }
			stack.emplace_at(locals_begin + 2, stack.pop_back<int64>());
		},
		[&](l_store_3) {
			if(info) { tabs(); print("l_store_3\n"); }
			stack.emplace_at(locals_begin + 3, stack.pop_back<int64>());
		},
		[&](f_store_0) {
			if(info) { tabs(); print("f_store_0\n"); }
			stack.emplace_at(locals_begin + 0, stack.pop_back<float>());
		},
		[&](f_store_1) {
			if(info) { tabs(); print("f_store_1\n"); }
			stack.emplace_at(locals_begin + 1, stack.pop_back<float>());
		},
		[&](f_store_2) {
			if(info) { tabs(); print("f_store_2\n"); }
			stack.emplace_at(locals_begin + 2, stack.pop_back<float>());
		},
		[&](f_store_3) {
			if(info) { tabs(); print("f_store_3\n"); }
			stack.emplace_at(locals_begin + 3, stack.pop_back<float>());
		},
		[&](d_store_0) {
			if(info) { tabs(); print("d_store_0\n"); }
			stack.emplace_at(locals_begin + 0, stack.pop_back<double>());
		},
		[&](d_store_1) {
			if(info) { tabs(); print("d_store_1\n"); }
			stack.emplace_at(locals_begin + 1, stack.pop_back<double>());
		},
		[&](d_store_2) {
			if(info) { tabs(); print("d_store_2\n"); }
			stack.emplace_at(locals_begin + 2, stack.pop_back<double>());
		},
		[&](d_store_3) {
			if(info) { tabs(); print("d_store_3\n"); }
			stack.emplace_at(locals_begin + 3, stack.pop_back<double>());
		},
		[&](a_store_0) {
			if(info) { tabs(); print("a_store_0\n"); }
			stack.emplace_at(locals_begin + 0, stack.pop_back<reference>());
		},
		[&](a_store_1) {
			if(info) { tabs(); print("a_store_1\n"); }
			stack.emplace_at(locals_begin + 1, stack.pop_back<reference>());
		},
		[&](a_store_2) {
			if(info) { tabs(); print("a_store_2\n"); }
			stack.emplace_at(locals_begin + 2, stack.pop_back<reference>());
		},
		[&](a_store_3) {
			if(info) { tabs(); print("a_store_3\n"); }
			stack.emplace_at(locals_begin + 3, stack.pop_back<reference>());
		},
		[&](i_a_store) {
			int32 value = stack.pop_back<int32>();
			if(info) { tabs(); print("i_a_store "); print(value); print("\n"); }
			return view_array.template operator()<int32>([&](int32& v) {
				v = value;
			});
		},
		[&](l_a_store) {
			int64 value = stack.pop_back<int64>();
			if(info) { tabs(); print("l_a_store "); print(value); print("\n"); }
			return view_array.template operator()<int64>([&](int64& v) {
				v = value;
			});
		},
		[&](f_a_store) {
			float value = stack.pop_back<float>();
			if(info) {
				tabs(); print("f_a_store ");
				//print(value); print("\n"); } // TODO
			}
			return view_array.template operator()<float>([&](float& v) {
				v = value;
			});
		},
		[&](d_a_store) {
			double value = stack.pop_back<double>();
			if(info) {
				tabs(); print("d_a_store ");
				//print(value); print("\n"); } // TODO
			}
			return view_array.template operator()<double>([&](double& v) {
				v = value;
			});
		},
		[&](a_a_store) {
			if(info) { tabs(); print("a_a_store\n"); }
			reference value = stack.pop_back<reference>();
			return view_array.template operator()<reference>([&](reference& v) {
				v = value;
			});
		},
		[&](b_a_store) {
			int32 value0 = stack.pop_back<int32>();
			int8 value = (int8) (uint16) (uint32) value0;
			if(info) { tabs(); print("b_a_store "); print(value); print("\n"); }
			return view_array.template operator()<int8>([&](int8& v) {
				v = value;
			});
		},
		[&](c_a_store) {
			int32 value0 = stack.pop_back<int32>();
			uint16 value = (uint16) (uint32) value0;
			if(info) { tabs(); print("c_a_store "); print(value); print("\n"); }
			return view_array.template operator() <uint16>([&](uint16& v) {
				v = value;
			});
		},
		[&](s_a_store) {
			int32 value0 = stack.pop_back<int32>();
			int16 value = (int16) (uint16) (uint32) value0;
			if(info) { tabs(); print("c_a_store "); print(value); print("\n"); }
			return view_array.template operator() <int16>([&](int16& v) {
				v = value;
			});
		},
		[](pop) {
			if(info) { tabs(); print("pop\n"); }
			stack.pop_back();
		},
		[](pop_2) {
			if(info) { tabs(); print("pop_2\n"); }
			stack.pop_back(2);
		},
		[](dup) {
			if(info) {
				tabs(); print("dup");
				print(" @"); print(stack.size()); print("\n");
			}
			stack.dup_cat_1();
		},
		[](dup_x1) {
			if(info) { tabs(); print("dup_x1\n"); }
			stack.dup_x1();
		},
		[](dup_x2) {
			if(info) { tabs(); print("dup_x2\n"); }
			stack.dup_x2();
		},
		[](dup_2) {
			if(info) { tabs(); print("dup_2\n"); }
			stack.dup2();
		},
		[](dup_2_x1) {
			if(info) { tabs(); print("dup_2_x1\n"); }
			stack.dup2_x1();
		},
		[](dup_2_x2) {
			if(info) { tabs(); print("dup_2_x2\n"); }
			stack.dup2_x2();
		},
		[](swap) {
			if(info) { tabs(); print("swap\n"); }
			stack.swap();
		},
		[](i_add) {
			if(info) { tabs(); print("i_add\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 + value2 });
		},
		[](l_add) {
			if(info) { tabs(); print("l_add\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 + value2 });
		},
		[](f_add) {
			if(info) { tabs(); print("f_add\n"); }
			float value2 = stack.pop_back<float>();
			float value1 = stack.pop_back<float>();
			stack.emplace_back(float{ value1 + value2 });
		},
		[](d_add) {
			if(info) { tabs(); print("d_add\n"); }
			double value2 = stack.pop_back<double>();
			double value1 = stack.pop_back<double>();
			stack.emplace_back(double{ value1 + value2 });
		},
		[](i_sub) {
			if(info) { tabs(); print("i_sub\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 - value2 });
		},
		[](l_sub) {
			if(info) { tabs(); print("l_sub\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 - value2 });
		},
		[](f_sub) {
			if(info) { tabs(); print("f_sub\n"); }
			float value2 = stack.pop_back<float>();
			float value1 = stack.pop_back<float>();
			stack.emplace_back(float{ value1 - value2 });
		},
		[](d_sub) {
			if(info) { tabs(); print("d_sub\n"); }
			double value2 = stack.pop_back<double>();
			double value1 = stack.pop_back<double>();
			stack.emplace_back(double{ value1 - value2 });
		},
		[](i_mul) {
			if(info) { tabs(); print("i_mul\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 * value2 });
		},
		[](l_mul) {
			if(info) { tabs(); print("l_mul\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 * value2 });
		},
		[](f_mul) {
			if(info) { tabs(); print("f_mul\n"); }
			float value2 = stack.pop_back<float>();
			float value1 = stack.pop_back<float>();
			stack.emplace_back(float{ value1 * value2 });
		},
		[](d_mul) {
			if(info) { tabs(); print("d_mul\n"); }
			double value2 = stack.pop_back<double>();
			double value1 = stack.pop_back<double>();
			stack.emplace_back(double{ value1 * value2 });
		},
		[](i_div) {
			if(info) { tabs(); print("i_div\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 / value2 });
		},
		[](l_div) {
			if(info) { tabs(); print("l_div\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 / value2 });
		},
		[](f_div) {
			if(info) { tabs(); print("f_div\n"); }
			float value2 = stack.pop_back<float>();
			float value1 = stack.pop_back<float>();
			stack.emplace_back(float{ value1 / value2 });
		},
		[](d_div) {
			if(info) { tabs(); print("d_div\n"); }
			double value2 = stack.pop_back<double>();
			double value1 = stack.pop_back<double>();
			stack.emplace_back(double{ value1 / value2 });
		},
		[](i_rem) {
			if(info) { tabs(); print("i_rem\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 - (value1 / value2) * value2 });
		},
		[](l_rem) {
			if(info) { tabs(); print("l_rem\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 - (value1 / value2) * value2 });
		},
		[](f_rem) { // TODO spec
			if(info) { tabs(); print("f_rem\n"); }
			float value2 = stack.pop_back<float>();
			float value1 = stack.pop_back<float>();
			stack.emplace_back(float{ value1 - (value1 / value2) * value2 });
		},
		[](d_rem) { // TODO spec
			if(info) { tabs(); print("d_rem\n"); }
			double value2 = stack.pop_back<double>();
			double value1 = stack.pop_back<double>();
			stack.emplace_back(double{ value1 - (value1 / value2) * value2 });
		},
		[](i_neg) {
			if(info) { tabs(); print("i_neg\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back(int32{ -value });
		},
		[](l_neg) {
			if(info) { tabs(); print("l_neg\n"); }
			int64 value = stack.pop_back<int64>();
			stack.emplace_back(int64{ -value });
		},
		[](f_neg) {
			if(info) { tabs(); print("f_neg\n"); }
			float value = stack.pop_back<float>();
			stack.emplace_back(float{ -value });
		},
		[](d_neg) {
			if(info) { tabs(); print("d_neg\n"); }
			double value = stack.pop_back<double>();
			stack.emplace_back(double{ -value });
		},
		[](i_sh_l) {
			if(info) { tabs(); print("i_sh_l\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 << (value2 & 0x1F) });
		},
		[](l_sh_l) {
			if(info) { tabs(); print("l_sh_l\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 << (value2 & 0x3F) });
		},
		[](i_sh_r) {
			if(info) { tabs(); print("i_sh_r\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 >> (value2 & 0x1F) });
		},
		[](l_sh_r) {
			if(info) { tabs(); print("l_sh_r\n"); }
			int32 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 >> (value2 & 0x3F) });
		},
		[](i_u_sh_r) {
			if(info) { tabs(); print("i_u_sh_r\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32 {
				int32(uint32(value1) >> (value2 & 0x1F))
			});
		},
		[](l_u_sh_r) {
			if(info) { tabs(); print("l_ush_r\n"); }
			int32 value2 = stack.pop_back<int32>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64 {
				int64(uint64(value1) >> (value2 & 0x3F))
			});
		},
		[](i_and) {
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			int32 result = value1 & value2;
			if(info) {
				tabs();
				print("i_and ");
				print(value1);
				print(" ");
				print(value2);
				print(" = ");
				print(result);
				print("\n");
			}
			stack.emplace_back(result);
		},
		[](l_and) {
			if(info) { tabs(); print("l_and\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 & value2 });
		},
		[](i_or) {
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			int32 result = value1 | value2;
			if(info) {
				tabs();
				print("i_and ");
				print(value1);
				print(" ");
				print(value2);
				print(" = ");
				print(result);
				print("\n");
			}
			stack.emplace_back(result);
		},
		[](l_or) {
			if(info) { tabs(); print("l_or\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 | value2 });
		},
		[](i_xor) {
			if(info) { tabs(); print("i_xor\n"); }
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			stack.emplace_back(int32{ value1 ^ value2 });
		},
		[](l_xor) {
			if(info) { tabs(); print("l_xor\n"); }
			int64 value2 = stack.pop_back<int64>();
			int64 value1 = stack.pop_back<int64>();
			stack.emplace_back(int64{ value1 ^ value2 });
		},
		[&](i_inc x) {
			if(info) {
				tabs();
				print("i_inc ");
				print(x.index);
				print(" ");
				print(x.value);
				print("\n");
			}
			stack.get<int32>(locals_begin + x.index) += x.value;
		},
		[](i_to_l) {
			if(info) { tabs(); print("i_to_l\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back((int64) value);
		},
		[](i_to_f) {
			if(info) { tabs(); print("i_to_f\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back((float) value);
		},
		[](i_to_d) {
			if(info) { tabs(); print("i_to_d\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back((double) value);
		},
		[](l_to_i) {
			if(info) { tabs(); print("l_to_i\n"); }
			int64 value = stack.pop_back<int64>();
			stack.emplace_back((int32) (value & 0xFFFFFFFF));
		},
		[](l_to_f) {
			if(info) { tabs(); print("l_to_f\n"); }
			int64 value = stack.pop_back<int64>();
			stack.emplace_back((float) value);
		},
		[](l_to_d) {
			if(info) { tabs(); print("l_to_d\n"); }
			int64 value = stack.pop_back<int64>();
			stack.emplace_back((double) value);
		},
		[](f_to_i) {
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
		},
		[](f_to_l) {
			if(info) { tabs(); print("f_to_l\n"); }
			float value = stack.pop_back<float>();
			stack.emplace_back((int64) value);
		},
		[](f_to_d) {
			if(info) { tabs(); print("f_to_d\n"); }
			float value = stack.pop_back<float>();
			stack.emplace_back((double) value);
		},
		[](d_to_i) {
			if(info) { tabs(); print("d_to_i\n"); }
			double value = stack.pop_back<double>();
			stack.emplace_back((int32) value);
		},
		[](d_to_l) {
			if(info) { tabs(); print("d_to_l\n"); }
			double value = stack.pop_back<double>();
			stack.emplace_back((int64) value);
		},
		[](d_to_f) {
			if(info) { tabs(); print("d_to_f\n"); }
			double value = stack.pop_back<double>();
			stack.emplace_back((float) value);
		},
		[](i_to_b) {
			if(info) { tabs(); print("i_to_b\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back((int32) (uint32) (int8) value);
		},
		[](i_to_c) {
			if(info) { tabs(); print("i_to_c\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back((int32) (uint32) (uint16) (uint32) value);
		},
		[](i_to_s) {
			if(info) { tabs(); print("i_to_s\n"); }
			int32 value = stack.pop_back<int32>();
			stack.emplace_back((int32) (uint32) (int16) value);
		},
		[](l_cmp) {
			if(info) { tabs(); print("l_cmp\n"); }
			int64 value_2 = stack.pop_back<int64>();
			int64 value_1 = stack.pop_back<int64>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			if(value_1 == value_2) result =  0;
			if(value_1 <  value_2) result = -1;
			stack.emplace_back(result);
		},
		[](f_cmp_l) {
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
		},
		[](f_cmp_g) {
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
		},
		[](d_cmp_l) {
			if(info) { tabs(); print("d_cmp_l\n"); }
			double value_2 = stack.pop_back<double>();
			double value_1 = stack.pop_back<double>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			else if(value_1 == value_2) result =  0;
			else if(value_1 <  value_2) result = -1;
			else { // NaN
				result = -1;
			}
			stack.emplace_back(result);
		},
		[](d_cmp_g) {
			if(info) { tabs(); print("d_cmp_g\n"); }
			double value_2 = stack.pop_back<double>();
			double value_1 = stack.pop_back<double>();
			int32 result;
			if(value_1 >  value_2) result =  1;
			else if(value_1 == value_2) result =  0;
			else if(value_1 <  value_2) result = -1;
			else { // NaN
				result = 1;
			}
			stack.emplace_back(result);
		},
		[&](if_eq x) {
			if(info) {
				tabs(); print("if_eq "); print(x.branch); print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value == 0) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_ne x) {
			if(info) {
				tabs(); print("if_ne "); print(x.branch); print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value != 0) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_lt x) {
			if(info) {
				tabs(); print("if_lt "); print(x.branch); print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value < 0) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_ge x) {
			if(info) {
				tabs(); print("if_ge "); print(x.branch); print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value >= 0) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_gt x) {
			if(info) {
				tabs(); print("if_gt "); print(x.branch); print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value > 0) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_le x) {
			if(info) {
				tabs(); print("if_le "); print(x.branch); print("\n");
			}
			int32 value = stack.pop_back<int32>();
			if(value <= 0) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_i_cmp_eq x) {
			if(info) {
				tabs(); print("if_i_cmp_eq "); print(x.branch); print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 == value2) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_i_cmp_ne x) {
			if(info) {
				tabs(); print("if_i_cmp_ne "); print(x.branch); print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 != value2) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_i_cmp_lt x) {
			if(info) {
				tabs(); print("if_i_cmp_lt "); print(x.branch); print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 < value2) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_i_cmp_ge x) {
			if(info) {
				tabs(); print("if_i_cmp_ge "); print(x.branch); print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 >= value2) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_i_cmp_gt x) {
			if(info) {
				tabs(); print("if_i_cmp_gt "); print(x.branch); print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 > value2) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_i_cmp_le x) {
			if(info) {
				tabs(); print("if_i_cmp_le "); print(x.branch); print("\n");
			}
			int32 value2 = stack.pop_back<int32>();
			int32 value1 = stack.pop_back<int32>();
			if(value1 <= value2) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_a_cmp_eq x) {
			if(info) {
				tabs(); print("if_a_cmp_eq "); print(x.branch); print("\n");
			}
			reference value2 = stack.pop_back<reference>();
			reference value1 = stack.pop_back<reference>();
			if(value1.object_ptr() == value2.object_ptr()) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_a_cmp_ne x) {
			if(info) {
				tabs(); print("if_a_cmp_ne "); print(x.branch); print("\n");
			}
			reference value2 = stack.pop_back<reference>();
			reference value1 = stack.pop_back<reference>();
			if(value1.object_ptr() != value2.object_ptr()) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](go_to x) {
			if(info) {
				tabs(); print("go_to "); print(x.branch); print("\n");
			}
			next_instruction_ptr = instrution_ptr + x.branch;
		},
		[&](jmp_sr x) {
			if(info) { tabs(); print("jmp_sr\n"); }
			uint32 address = (uint32) (uint64) next_instruction_ptr;
			stack.emplace_back((int32)address);
			next_instruction_ptr = instrution_ptr + x.branch;
		},
		[&](return_sr x) {
			if(info) { tabs(); print("return_sr\n"); }
			uint32 address = stack.get<int32>(x.index);
			next_instruction_ptr = m.code().iterator() + address;
		},
		[&](table_switch x) {
			if(info) { tabs(); print("table_switch\n"); }
			/* The index must be of type int and is popped from the operand
			   stack. */
			int32 index = stack.pop_back<int32>();

			/* If index is less than low or index is greater than high, ... */
			if(index < x.low || index > x.hight) {
				/* ... then a target address is calculated by adding default to
				   the address of the opcode of this tableswitch instruction */
				next_instruction_ptr = instrution_ptr + x._default;
			}
			else {
				/* Otherwise, the offset at position index - low of the jump
				table is extracted */
				uint32 position = index - x.low;
				int32 offset = x.offsets[position];
				/* The target address is calculated by adding that offset to the
				address of the opcode of this tableswitch instruction.
				Execution then continues at the target address. */
				next_instruction_ptr = instrution_ptr + offset;
			}
		},
		[&](i_return) {
			int32 result = stack.back<int32>();
			m.return_type().view([&]<typename RetType>(RetType) {
				if constexpr(same_as<RetType, class_file::z>) {
					result = result & 1;
				}
				if constexpr(same_as<RetType, class_file::b>) {
					result = uint8(result);
				}
				if constexpr(same_as<RetType, class_file::c>) {
					result = uint16(result);
				}
				if constexpr(same_as<RetType, class_file::s>) {
					result = int16(result);
				}
			});
			stack.pop_back_until(locals_begin);
			stack.emplace_back(result);
			if(info) {
				tabs();
				print("i_return ");
				print(result);
				print("\n");
			}
			return loop_action::stop;
		},
		[&](l_return) {
			if(info) { tabs(); print("l_return\n"); }
			int64 result = stack.pop_back<int64>();
			stack.pop_back_until(locals_begin);
			stack.emplace_back(result);
			return loop_action::stop;
		},
		[&](f_return) {
			if(info) { tabs(); print("f_return\n"); }
			float result = stack.pop_back<float>();
			stack.pop_back_until(locals_begin);
			stack.emplace_back(result);
			return loop_action::stop;
		},
		[&](d_return) {
			if(info) { tabs(); print("d_return\n"); }
			double result = stack.pop_back<double>();
			stack.pop_back_until(locals_begin);
			stack.emplace_back(result);
			return loop_action::stop;
		},
		[&](a_return) {
			if(info) { tabs(); print("a_return\n"); }
			reference result = stack.pop_back<reference>();
			stack.pop_back_until(locals_begin);
			stack.emplace_back(move(result));
			return loop_action::stop;
		},
		[&](_return) {
			if(info) { tabs(); print("return\n"); }
			stack.pop_back_until(locals_begin);
			return loop_action::stop;
		},
		[&](get_static x) {
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
				print(" @");
				print(stack.size());
				print("\n");
			}
			class_and_declared_static_field_index class_and_field_index
				= c.get_static_field_index(x.index);
			
			class_and_field_index._class.view(
				class_and_field_index.field_index,
				[](auto& field_value) {
					stack.emplace_back(field_value);
				}
			);
		},
		[&](put_static x) {
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
			class_and_declared_static_field_index class_and_field_index
				= c.get_static_field_index(x.index);
			class_and_field_index._class.view(
				class_and_field_index.field_index,
				[]<typename FieldType>(FieldType& field_value) {
					field_value = stack.pop_back<FieldType>();
				}
			);
		},
		[&](get_field x) {
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

			reference ref = stack.pop_back<reference>();
			if(ref.is_null()) {
				thrown = create_null_pointer_exception();
				return handle_thrown();
			}

			field_index_and_stack_size field_index_and_stack_size
				= c.get_resolved_instance_field_index(x.index);
			ref->view(
				field_index_and_stack_size.field_index,
				[&](auto& field_value) {
					stack.emplace_back(field_value);
				}
			);
			return loop_action::next;
		},
		[&](put_field x) {
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

			field_index_and_stack_size field_index_and_stack_size
				= c.get_resolved_instance_field_index(x.index);
			
			reference ref = move(stack.get<reference>(
				stack.size() - 1 - field_index_and_stack_size.stack_size
			));
			if(ref.is_null()) {
				thrown = create_null_pointer_exception();
				return handle_thrown();
			}
			ref->view(
				field_index_and_stack_size.field_index,
				[&]<typename FieldType>(FieldType& field_value) {
					field_value = stack.pop_back<FieldType>();
				}
			);
			stack.pop_back<reference>();
			return loop_action::next;
		},
		[&](instr::invoke_virtual x) {
			::invoke_virtual(x.index, c);
			return handle_thrown();
		},
		[&](instr::invoke_special x) {
			::invoke_special(x.index, c);
			return handle_thrown();
		},
		[&](instr::invoke_static x) {
			::invoke_static(x.index, c);
			return handle_thrown();
		},
		[&](instr::invoke_interface x) {
			::invoke_interface(x.index, c);
			return handle_thrown();
		},
		[&](instr::invoke_dynamic x) {
			::invoke_dynamic(x.index, c);
			return handle_thrown();
		},
		[&](_new x) {
			if(info) {
				tabs(); print("new ");
				cc::utf8 name = c.utf8_constant(
					c.class_constant(x.index).name_index
				);
				print(name);
				print(" @");
				print(stack.size());
				print("\n");
			}
			_class& c0 = c.get_resolved_class(x.index);
			stack.emplace_back(create_object(c0));
		},
		[&](instr::new_array x) {
			::new_array(/* c, */ x.type);
		},
		[&](a_new_array x) {
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
		},
		[&](instr::array_length) {
			if(info) { tabs(); print("array_length\n"); }
			reference ref = stack.pop_back<reference>();
			if(ref.is_null()) {
				thrown = create_null_pointer_exception();
				return handle_thrown();
			}
			stack.emplace_back(int32{ ::array_length(ref) });
			return loop_action::next;
		},
		[&](a_throw) {
			if(info) { tabs(); print("a_throw\n"); }

			reference ref = move(stack.pop_back<reference>());
			if(ref.is_null()) {
				ref = create_null_pointer_exception();
			}
			thrown = move(ref);
			return handle_thrown();
		},
		[&](class_file::attribute::code::instruction::check_cast x) {
			::check_cast(c, x.index);
			return handle_thrown();
		},
		[&](class_file::attribute::code::instruction::instance_of x) {
			::instance_of(c, x.index);
		},
		[&](monitor_enter) {
			reference ref = stack.pop_back<reference>();
			if(ref.is_null()) {
				ref = create_null_pointer_exception();
				return handle_thrown();
			}
			ref->lock();
			return loop_action::next;
		},
		[&](monitor_exit) {
			reference ref = stack.pop_back<reference>();
			if(ref.is_null()) {
				ref = create_null_pointer_exception();
				return handle_thrown();
			}
			ref->unlock();
			return loop_action::next;
		},
		[&](if_null x) {
			if(info) {
				tabs(); print("if_null "); print(x.branch); print("\n");
			}
			reference ref = stack.pop_back<reference>();
			if(ref.is_null()) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](if_non_null x) {
			if(info) {
				tabs(); print("if_non_null "); print(x.branch); print("\n");
			}
			reference ref = stack.pop_back<reference>();
			if(!ref.is_null()) {
				next_instruction_ptr = instrution_ptr + x.branch;
			}
		},
		[&](go_to_w x) {
			if(info) {
				tabs(); print("go_to_w "); print(x.branch); print("\n");
			}
			next_instruction_ptr = instrution_ptr + x.branch;
		},
		[&](jmp_sr_w x) {
			if(info) { tabs(); print("jmp_sr_w\n"); }
			uint32 address = (uint32) (uint64) next_instruction_ptr;
			stack.emplace_back((int32)address);
			next_instruction_ptr = instrution_ptr + x.branch;
		},
		[&](uint8 x) {
			if(info) tabs();
			print("unknown instruction ");
			print(x);
			abort();
		},
		[&](auto x) {
			on_unimplemented_instruction(decltype(x)::code);
		}
		}.then([](loop_action action = loop_action::next) {
			return action;
		})(x0);
	}, m.code().size());
}