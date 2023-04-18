#include "execution/info.hpp"
#include "execution/stack.hpp"

#include "class.hpp"
#include "method.hpp"
#include "array.hpp"
#include "reference.hpp"
#include "object.hpp"
#include "lib/java/lang/null_pointer_exception.hpp"
#include "lib/java/lang/index_out_of_bounds_exception.hpp"

#include "./ldc.hpp"
#include "./invoke_dynamic.hpp"
#include "./invoke_interface.hpp"
#include "./invoke_special.hpp"
#include "./invoke_static.hpp"
#include "./invoke_virtual.hpp"
#include "./check_cast.hpp"
#include "./instance_of.hpp"
#include "./new_array.hpp"

#include <class_file/attribute/code/instruction.hpp>

#include <posix/math.hpp>

#include <loop_action.hpp>

namespace instr = class_file::attribute::code::instruction;

struct execute_instruction {
	method& m;
	_class& c;
	const uint8* const instruction_ptr;
	const uint8*& next_instruction_ptr;

	const nuint locals_begin;
	const nuint stack_begin;
	reference& thrown;

	[[nodiscard]] loop_action handle_thrown(reference thrown) {
		if(info) {
			tabs();
			print::out("handling ", thrown._class().name(), "\n");
		}
		_class& thrown_class = thrown->_class();

		auto& exception_handlers = m.exception_handlers();

		for(class_file::attribute::code::exception_handler handler
			: exception_handlers.as_span()
		) {
			uint32 pc = instruction_ptr - m.code().iterator();
			bool in_range = pc >= handler.start_pc && pc < handler.end_pc;
			if(!in_range) {
				continue;
			}

			expected<_class&, reference> possible_catch_class
				= c.try_get_resolved_class(handler.catch_type);
			
			if(possible_catch_class.is_unexpected()) {
				print::err(
					"couldn't load catch class while handling throwable\n"
				);
				posix::abort();
			}

			_class& catch_class = possible_catch_class.get_expected();

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
		if(info) {
			tabs();
			print::out("didn't find any exception handlers\n");
		}
		stack.pop_back_until(locals_begin);
		this->thrown = move(thrown);
		return loop_action::stop;
	}

	template<typename E, typename Handler>
	loop_action view_array(Handler&& handler) {
		int32 element_index = stack.pop_back<int32>();
		reference array_ref = stack.pop_back<reference>();
		if(array_ref.is_null()) {
			expected<reference, reference> possible_npe
				= try_create_null_pointer_exception();
			return handle_thrown(move(possible_npe.get()));
		}
		int32 len = ::array_length(array_ref);
		if(element_index < 0 || element_index >= len) {
			expected<reference, reference> possible_ioobe
				= try_create_index_of_of_bounds_exception();
			return handle_thrown(move(possible_ioobe.get()));
		}
		E* ptr = array_data<E>(array_ref);
		handler(ptr[element_index]);
		return loop_action::next;
	};

	void operator () (instr::nop) {}

	void operator () (instr::a_const_null) {
		if(info) { tabs(); print::out("a_const_null\n"); }
		stack.emplace_back(reference{});
	}

	void operator () (instr::i_const_m1) {
		if(info) { tabs(); print::out("i_const_m1\n"); }
		stack.emplace_back(int32{ -1 });
	}
	void operator () (instr::i_const_0) {
		if(info) { tabs(); print::out("i_const_0\n"); }
		stack.emplace_back(int32{ 0 });
	}
	void operator () (instr::i_const_1) {
		if(info) { tabs(); print::out("i_const_1\n"); }
		stack.emplace_back(int32{ 1 });
	}
	void operator () (instr::i_const_2) {
		if(info) { tabs(); print::out("i_const_2\n"); }
		stack.emplace_back(int32{ 2 });
	}
	void operator () (instr::i_const_3) {
		if(info) { tabs(); print::out("i_const_3\n"); }
		stack.emplace_back(int32{ 3 });
	}
	void operator () (instr::i_const_4) {
		if(info) { tabs(); print::out("i_const_4\n"); }
		stack.emplace_back(int32{ 4 });
	}
	void operator () (instr::i_const_5) {
		if(info) {
			tabs(); print::out("i_const_5 @", stack.size(), "\n");
		}
		stack.emplace_back(int32{ 5 });
	}
	void operator () (instr::l_const_0) {
		if(info) {
			tabs(); print::out("l_const_0 @", stack.size(), "\n");
		}
		stack.emplace_back(int64{ 0 });
	}
	void operator () (instr::l_const_1) {
		if(info) { tabs(); print::out("l_const_1\n"); }
		stack.emplace_back(int64{ 1 });
	}
	void operator () (instr::f_const_0) {
		if(info) { tabs(); print::out("f_const_0\n"); }
		stack.emplace_back(float{ 0.0F });
	}
	void operator () (instr::f_const_1) {
		if(info) { tabs(); print::out("f_const_1\n"); }
		stack.emplace_back(float{ 1.0F });
	}
	void operator () (instr::f_const_2) {
		if(info) { tabs(); print::out("f_const_2\n"); }
		stack.emplace_back(float{ 2.0F });
	}
	void operator () (instr::d_const_0) {
		if(info) { tabs(); print::out("d_const_0\n"); }
		stack.emplace_back(double{ 0.0 });
	}
	void operator () (instr::d_const_1) {
		if(info) { tabs(); print::out("d_const_1\n"); }
		stack.emplace_back(double{ 1.0 });
	}
	void operator () (instr::bi_push x) {
		if(info) {
			tabs(); print::out("bi_push ", x.value, " @", stack.size(), "\n");
		}
		stack.emplace_back(int32{ x.value });
	}
	void operator () (instr::si_push x) {
		if(info) {
			tabs(); print::out("si_push ", x.value, "\n");
		}
		stack.emplace_back(int32{ x.value });
	}
	loop_action operator () (
		same_as_any<
			class_file::attribute::code::instruction::ldc,
			class_file::attribute::code::instruction::ldc_w
		> auto x
	) {
		optional<reference> possible_throwable = ::try_ldc(x.index, c);
		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.get());
		}
		return loop_action::next;
	}
	void operator () (class_file::attribute::code::instruction::ldc_2_w x) {
		::ldc_2_w(x.index, c);
	}
	void operator () (instr::i_load x) {
		int32 value = stack.get<int32>(locals_begin + x.index);
		if(info) {
			tabs();
			print::out("i_load ", x.index, " ", value, "\n");
		}
		stack.emplace_back(value);
	}
	void operator () (instr::l_load x) {
		if(info) {
			tabs();
			print::out("l_load ", x.index, "\n");
		}
		stack.emplace_back(stack.get<int64>(locals_begin + x.index));
	}
	void operator () (instr::f_load x) {
		if(info) {
			tabs();
			print::out("f_load ", x.index, "\n");
		}
		stack.emplace_back(stack.get<float>(locals_begin + x.index));
	}
	void operator () (instr::d_load x) {
		if(info) {
			tabs();
			print::out("d_load ", x.index, "\n");
		}
		stack.emplace_back(stack.get<double>(locals_begin + x.index));
	}
	void operator () (instr::a_load x) {
		if(info) {
			tabs();
			print::out("a_load ", x.index, "\n");
		}
		stack.emplace_back(stack.get<reference>(locals_begin + x.index));
	}
	void operator () (instr::i_load_0) {
		int32 value = stack.get<int32>(locals_begin + 0);
		if(info) {
			tabs();
			print::out("i_load_0 ", value, "\n");
		}
		stack.emplace_back(value);
	}
	void operator () (instr::i_load_1) {
		int32 value = stack.get<int32>(locals_begin + 1);
		if(info) {
			tabs();
			print::out("i_load_1 ", value, "\n");
		}
		stack.emplace_back(value);
	}
	void operator () (instr::i_load_2) {
		int32 value = stack.get<int32>(locals_begin + 2);
		if(info) {
			tabs();
			print::out("i_load_2 ", value, "\n");
		}
		stack.emplace_back(value);
	}
	void operator () (instr::i_load_3) {
		int32 value = stack.get<int32>(locals_begin + 3);
		if(info) {
			tabs();
			print::out("i_load_3 ", value, "\n");
		}
		stack.emplace_back(value);
	}
	void operator () (instr::l_load_0) {
		if(info) { tabs(); print::out("l_load_0\n"); }
		stack.emplace_back(stack.get<int64>(locals_begin + 0));
	}
	void operator () (instr::l_load_1) {
		if(info) { tabs(); print::out("l_load_1\n"); }
		stack.emplace_back(stack.get<int64>(locals_begin + 1));
	}
	void operator () (instr::l_load_2) {
		if(info) { tabs(); print::out("l_load_2\n"); }
		stack.emplace_back(stack.get<int64>(locals_begin + 2));
	}
	void operator () (instr::l_load_3) {
		if(info) { tabs(); print::out("l_load_3\n"); }
		stack.emplace_back(stack.get<int64>(locals_begin + 3));
	}
	void operator () (instr::f_load_0) {
		if(info) { tabs(); print::out("f_load_0\n"); }
		stack.emplace_back(stack.get<float>(locals_begin + 0));
	}
	void operator () (instr::f_load_1) {
		if(info) { tabs(); print::out("f_load_1\n"); }
		stack.emplace_back(stack.get<float>(locals_begin + 1));
	}
	void operator () (instr::f_load_2) {
		if(info) { tabs(); print::out("f_load_2\n"); }
		stack.emplace_back(stack.get<float>(locals_begin + 2));
	}
	void operator () (instr::f_load_3) {
		if(info) { tabs(); print::out("f_load_3\n"); }
		stack.emplace_back(stack.get<float>(locals_begin + 3));
	}
	void operator () (instr::d_load_0) {
		if(info) { tabs(); print::out("d_load_0\n"); }
		stack.emplace_back(stack.get<double>(locals_begin + 0));
	}
	void operator () (instr::d_load_1) {
		if(info) { tabs(); print::out("d_load_1\n"); }
		stack.emplace_back(stack.get<double>(locals_begin + 1));
	}
	void operator () (instr::d_load_2) {
		if(info) { tabs(); print::out("d_load_2\n"); }
		stack.emplace_back(stack.get<double>(locals_begin + 2));
	}
	void operator () (instr::d_load_3) {
		if(info) { tabs(); print::out("d_load_3\n"); }
		stack.emplace_back(stack.get<double>(locals_begin + 3));
	}
	void operator () (instr::a_load_0) {
		reference ref = stack.get<reference>(locals_begin + 0);
		if(info) {
			tabs(); print::out("a_load_0 ");
			if(!ref.is_null()) {
				print::out(ref._class().name());
			}
			print::out(" @");
			print::out.hex((uint64) ref.object_ptr());
			print::out("\n");
		}
		stack.emplace_back(move(ref));
	}
	void operator () (instr::a_load_1) {
		if(info) { tabs(); print::out("a_load_1\n"); }
		reference ref = stack.get<reference>(locals_begin + 1);
		stack.emplace_back(move(ref));
	}
	void operator () (instr::a_load_2) {
		reference ref = stack.get<reference>(locals_begin + 2);
		if(info) {
			tabs(); print::out("a_load_2 ");
			if(!ref.is_null()) {
				print::out(ref._class().name());
			}
			print::out(" @");
			print::out.hex((uint64) ref.object_ptr());
			print::out("\n");
		}
		stack.emplace_back(move(ref));
	}
	void operator () (instr::a_load_3) {
		if(info) { tabs(); print::out("a_load_3\n"); }
		reference ref = stack.get<reference>(locals_begin + 3);
		stack.emplace_back(move(ref));
	}
	loop_action operator () (instr::i_a_load) {
		if(info) { tabs(); print::out("i_a_load\n"); }
		return view_array<int32>([&](int32& v) {
			stack.emplace_back(v);
		});
	}
	loop_action operator () (instr::l_a_load) {
		if(info) { tabs(); print::out("l_a_load\n"); }
		return view_array<int64>([&](int64& v) {
			stack.emplace_back(v);
		});
	}
	loop_action operator () (instr::f_a_load) {
		if(info) { tabs(); print::out("f_a_load\n"); }
		return view_array<float>([&](float& v) {
			stack.emplace_back(v);
		});
	}
	loop_action operator () (instr::d_a_load) {
		if(info) { tabs(); print::out("d_a_load\n"); }
		return view_array<double>([&](double& v) {
			stack.emplace_back(v);
		});
	}
	loop_action operator () (instr::a_a_load) {
		if(info) { tabs(); print::out("a_a_load\n"); }
		return view_array<reference>([&](reference& v) {
			stack.emplace_back(v);
		});
	}
	loop_action operator () (instr::b_a_load) {
		if(info) { tabs(); print::out("b_a_load\n"); }
		return view_array<int8>([&](int8& v) {
			stack.emplace_back(v);
		});
	}
	loop_action operator () (instr::c_a_load) {
		if(info) { tabs(); print::out("c_a_load\n"); }
		return view_array<uint16>([&](uint16& v) {
			stack.emplace_back(v);
		});
	}
	loop_action operator () (instr::s_a_load) {
		if(info) { tabs(); print::out("s_a_load\n"); }
		return view_array<int16>([&](int16& v) {
			stack.emplace_back(v);
		});
	}
	void operator () (instr::i_store x) {
		int32 value = stack.pop_back<int32>();
		if(info) {
			tabs(); print::out("i_store ", x.index, " ", value, "\n");
		}
		stack.emplace_at(locals_begin + x.index, value);
	}
	void operator () (instr::l_store x) {
		if(info) {
			tabs(); print::out("l_store ", x.index, "\n");
		}
		stack.emplace_at(locals_begin + x.index, stack.pop_back<int64>());
	}
	void operator () (instr::f_store x) {
		if(info) {
			tabs(); print::out("f_store ", x.index, "\n");
		}
		stack.emplace_at(locals_begin + x.index, stack.pop_back<float>());
	}
	void operator () (instr::d_store x) {
		if(info) {
			tabs(); print::out("d_store ", x.index, "\n");
		}
		stack.emplace_at(locals_begin + x.index, stack.pop_back<double>());
	}
	void operator () (instr::a_store x) {
		if(info) {
			tabs(); print::out("a_store ", x.index, "\n");
		}
		stack.emplace_at(
			locals_begin + x.index, stack.pop_back<reference>()
		);
	}
	void operator () (instr::i_store_0) {
		int32 value = stack.pop_back<int32>();
		if(info) { tabs(); print::out("i_store_0 ", value, "\n"); }
		stack.emplace_at(locals_begin + 0, value);
	}
	void operator () (instr::i_store_1) {
		int32 value = stack.pop_back<int32>();
		if(info) { tabs(); print::out("i_store_1 ", value, "\n"); }
		stack.emplace_at(locals_begin + 1, value);
	}
	void operator () (instr::i_store_2) {
		int32 value = stack.pop_back<int32>();
		if(info) { tabs(); print::out("i_store_2 ", value, "\n"); }
		stack.emplace_at(locals_begin + 2, value);
	}
	void operator () (instr::i_store_3) {
		int32 value = stack.pop_back<int32>();
		if(info) { tabs(); print::out("i_store_3 ", value, "\n"); }
		stack.emplace_at(locals_begin + 3, value);
	}
	void operator () (instr::l_store_0) {
		if(info) { tabs(); print::out("l_store_0\n"); }
		stack.emplace_at(locals_begin + 0, stack.pop_back<int64>());
	}
	void operator () (instr::l_store_1) {
		if(info) { tabs(); print::out("l_store_1\n"); }
		stack.emplace_at(locals_begin + 1, stack.pop_back<int64>());
	}
	void operator () (instr::l_store_2) {
		if(info) { tabs(); print::out("l_store_2\n"); }
		stack.emplace_at(locals_begin + 2, stack.pop_back<int64>());
	}
	void operator () (instr::l_store_3) {
		if(info) { tabs(); print::out("l_store_3\n"); }
		stack.emplace_at(locals_begin + 3, stack.pop_back<int64>());
	}
	void operator () (instr::f_store_0) {
		if(info) { tabs(); print::out("f_store_0\n"); }
		stack.emplace_at(locals_begin + 0, stack.pop_back<float>());
	}
	void operator () (instr::f_store_1) {
		if(info) { tabs(); print::out("f_store_1\n"); }
		stack.emplace_at(locals_begin + 1, stack.pop_back<float>());
	}
	void operator () (instr::f_store_2) {
		if(info) { tabs(); print::out("f_store_2\n"); }
		stack.emplace_at(locals_begin + 2, stack.pop_back<float>());
	}
	void operator () (instr::f_store_3) {
		if(info) { tabs(); print::out("f_store_3\n"); }
		stack.emplace_at(locals_begin + 3, stack.pop_back<float>());
	}
	void operator () (instr::d_store_0) {
		if(info) { tabs(); print::out("d_store_0\n"); }
		stack.emplace_at(locals_begin + 0, stack.pop_back<double>());
	}
	void operator () (instr::d_store_1) {
		if(info) { tabs(); print::out("d_store_1\n"); }
		stack.emplace_at(locals_begin + 1, stack.pop_back<double>());
	}
	void operator () (instr::d_store_2) {
		if(info) { tabs(); print::out("d_store_2\n"); }
		stack.emplace_at(locals_begin + 2, stack.pop_back<double>());
	}
	void operator () (instr::d_store_3) {
		if(info) { tabs(); print::out("d_store_3\n"); }
		stack.emplace_at(locals_begin + 3, stack.pop_back<double>());
	}
	void operator () (instr::a_store_0) {
		if(info) { tabs(); print::out("a_store_0\n"); }
		stack.emplace_at(locals_begin + 0, stack.pop_back<reference>());
	}
	void operator () (instr::a_store_1) {
		if(info) { tabs(); print::out("a_store_1\n"); }
		stack.emplace_at(locals_begin + 1, stack.pop_back<reference>());
	}
	void operator () (instr::a_store_2) {
		if(info) { tabs(); print::out("a_store_2\n"); }
		stack.emplace_at(locals_begin + 2, stack.pop_back<reference>());
	}
	void operator () (instr::a_store_3) {
		if(info) { tabs(); print::out("a_store_3\n"); }
		stack.emplace_at(locals_begin + 3, stack.pop_back<reference>());
	}
	loop_action operator () (instr::i_a_store) {
		int32 value = stack.pop_back<int32>();
		if(info) { tabs(); print::out("i_a_store ", value, "\n"); }
		return view_array<int32>([&](int32& v) {
			v = value;
		});
	}
	loop_action operator () (instr::l_a_store) {
		int64 value = stack.pop_back<int64>();
		if(info) { tabs(); print::out("l_a_store ", value, "\n"); }
		return view_array<int64>([&](int64& v) {
			v = value;
		});
	}
	loop_action operator () (instr::f_a_store) {
		float value = stack.pop_back<float>();
		if(info) { tabs(); print::out("f_a_store ", value, "\n"); }
		return view_array<float>([&](float& v) {
			v = value;
		});
	}
	loop_action operator () (instr::d_a_store) {
		double value = stack.pop_back<double>();
		if(info) { tabs(); print::out("d_a_store ", value, "\n"); }
		return view_array<double>([&](double& v) {
			v = value;
		});
	}
	loop_action operator () (instr::a_a_store) {
		if(info) { tabs(); print::out("a_a_store\n"); }
		reference value = stack.pop_back<reference>();
		return view_array<reference>([&](reference& v) {
			v = value;
		});
	}
	loop_action operator () (instr::b_a_store) {
		int32 value0 = stack.pop_back<int32>();
		int8 value = (int8) (uint16) (uint32) value0;
		if(info) { tabs(); print::out("b_a_store ", value, "\n"); }
		return view_array<int8>([&](int8& v) {
			v = value;
		});
	}
	loop_action operator () (instr::c_a_store) {
		int32 value0 = stack.pop_back<int32>();
		uint16 value = (uint16) (uint32) value0;
		if(info) { tabs(); print::out("c_a_store ", value, "\n"); }
		return view_array<uint16>([&](uint16& v) {
			v = value;
		});
	}
	loop_action operator () (instr::s_a_store) {
		int32 value0 = stack.pop_back<int32>();
		int16 value = (int16) (uint16) (uint32) value0;
		if(info) { tabs(); print::out("c_a_store ", value, "\n"); }
		return view_array<int16>([&](int16& v) {
			v = value;
		});
	}
	void operator () (instr::pop) {
		if(info) { tabs(); print::out("pop\n"); }
		stack.pop_back();
	}
	void operator () (instr::pop_2) {
		if(info) { tabs(); print::out("pop_2\n"); }
		stack.pop_back(2);
	}
	void operator () (instr::dup) {
		if(info) { tabs(); print::out("dup @", stack.size(), "\n"); }
		stack.dup_cat_1();
	}
	void operator () (instr::dup_x1) {
		if(info) { tabs(); print::out("dup_x1\n"); }
		stack.dup_x1();
	}
	void operator () (instr::dup_x2) {
		if(info) { tabs(); print::out("dup_x2\n"); }
		stack.dup_x2();
	}
	void operator () (instr::dup_2) {
		if(info) { tabs(); print::out("dup_2\n"); }
		stack.dup2();
	}
	void operator () (instr::dup_2_x1) {
		if(info) { tabs(); print::out("dup_2_x1\n"); }
		stack.dup2_x1();
	}
	void operator () (instr::dup_2_x2) {
		if(info) { tabs(); print::out("dup_2_x2\n"); }
		stack.dup2_x2();
	}
	void operator () (instr::swap) {
		if(info) { tabs(); print::out("swap\n"); }
		stack.swap();
	}
	void operator () (instr::i_add) {
		if(info) { tabs(); print::out("i_add\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		stack.emplace_back(int32{ value1 + value2 });
	}
	void operator () (instr::l_add) {
		if(info) { tabs(); print::out("l_add\n"); }
		int64 value2 = stack.pop_back<int64>();
		int64 value1 = stack.pop_back<int64>();
		stack.emplace_back(int64{ value1 + value2 });
	}
	void operator () (instr::f_add) {
		if(info) { tabs(); print::out("f_add\n"); }
		float value2 = stack.pop_back<float>();
		float value1 = stack.pop_back<float>();
		stack.emplace_back(float{ value1 + value2 });
	}
	void operator () (instr::d_add) {
		if(info) { tabs(); print::out("d_add\n"); }
		double value2 = stack.pop_back<double>();
		double value1 = stack.pop_back<double>();
		stack.emplace_back(double{ value1 + value2 });
	}
	void operator () (instr::i_sub) {
		if(info) { tabs(); print::out("i_sub\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		stack.emplace_back(int32{ value1 - value2 });
	}
	void operator () (instr::l_sub) {
		if(info) { tabs(); print::out("l_sub\n"); }
		int64 value2 = stack.pop_back<int64>();
		int64 value1 = stack.pop_back<int64>();
		stack.emplace_back(int64{ value1 - value2 });
	}
	void operator () (instr::f_sub) {
		if(info) { tabs(); print::out("f_sub\n"); }
		float value2 = stack.pop_back<float>();
		float value1 = stack.pop_back<float>();
		stack.emplace_back(float{ value1 - value2 });
	}
	void operator () (instr::d_sub) {
		if(info) { tabs(); print::out("d_sub\n"); }
		double value2 = stack.pop_back<double>();
		double value1 = stack.pop_back<double>();
		stack.emplace_back(double{ value1 - value2 });
	}
	void operator () (instr::i_mul) {
		if(info) { tabs(); print::out("i_mul\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		stack.emplace_back(int32{ value1 * value2 });
	}
	void operator () (instr::l_mul) {
		if(info) { tabs(); print::out("l_mul\n"); }
		int64 value2 = stack.pop_back<int64>();
		int64 value1 = stack.pop_back<int64>();
		stack.emplace_back(int64{ value1 * value2 });
	}
	void operator () (instr::f_mul) {
		if(info) { tabs(); print::out("f_mul\n"); }
		float value2 = stack.pop_back<float>();
		float value1 = stack.pop_back<float>();
		stack.emplace_back(float{ value1 * value2 });
	}
	void operator () (instr::d_mul) {
		if(info) { tabs(); print::out("d_mul\n"); }
		double value2 = stack.pop_back<double>();
		double value1 = stack.pop_back<double>();
		stack.emplace_back(double{ value1 * value2 });
	}
	void operator () (instr::i_div) {
		if(info) { tabs(); print::out("i_div\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		stack.emplace_back(int32{ value1 / value2 });
	}
	void operator () (instr::l_div) {
		if(info) { tabs(); print::out("l_div\n"); }
		int64 value2 = stack.pop_back<int64>();
		int64 value1 = stack.pop_back<int64>();
		stack.emplace_back(int64{ value1 / value2 });
	}
	void operator () (instr::f_div) {
		if(info) { tabs(); print::out("f_div\n"); }
		float value2 = stack.pop_back<float>();
		float value1 = stack.pop_back<float>();
		stack.emplace_back(float{ value1 / value2 });
	}
	void operator () (instr::d_div) {
		if(info) { tabs(); print::out("d_div\n"); }
		double value2 = stack.pop_back<double>();
		double value1 = stack.pop_back<double>();
		stack.emplace_back(double{ value1 / value2 });
	}
	void operator () (instr::i_rem) {
		if(info) { tabs(); print::out("i_rem "); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		int32 result = int32{ value1 - (value1 / value2) * value2 };
		if(info) { print::out(value1," % ", value2, " = ", result); }
		stack.emplace_back(result);
	}
	void operator () (instr::l_rem) {
		if(info) { tabs(); print::out("l_rem\n"); }
		int64 value2 = stack.pop_back<int64>();
		int64 value1 = stack.pop_back<int64>();
		stack.emplace_back(int64{ value1 - (value1 / value2) * value2 });
	}
	void operator () (instr::f_rem) { // TODO spec
		if(info) { tabs(); print::out("f_rem\n"); }
		float value2 = stack.pop_back<float>();
		float value1 = stack.pop_back<float>();
		stack.emplace_back(float{ value1 - (value1 / value2) * value2 });
	}
	void operator () (instr::d_rem) { // TODO spec
		if(info) { tabs(); print::out("d_rem\n"); }
		double value2 = stack.pop_back<double>();
		double value1 = stack.pop_back<double>();
		stack.emplace_back(double{ value1 - (value1 / value2) * value2 });
	}
	void operator () (instr::i_neg) {
		if(info) { tabs(); print::out("i_neg\n"); }
		int32 value = stack.pop_back<int32>();
		stack.emplace_back(int32{ -value });
	}
	void operator () (instr::l_neg) {
		if(info) { tabs(); print::out("l_neg\n"); }
		int64 value = stack.pop_back<int64>();
		stack.emplace_back(int64{ -value });
	}
	void operator () (instr::f_neg) {
		if(info) { tabs(); print::out("f_neg\n"); }
		float value = stack.pop_back<float>();
		stack.emplace_back(float{ -value });
	}
	void operator () (instr::d_neg) {
		if(info) { tabs(); print::out("d_neg\n"); }
		double value = stack.pop_back<double>();
		stack.emplace_back(double{ -value });
	}
	void operator () (instr::i_sh_l) {
		if(info) { tabs(); print::out("i_sh_l\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		stack.emplace_back(int32{ value1 << (value2 & 0x1F) });
	}
	void operator () (instr::l_sh_l) {
		if(info) { tabs(); print::out("l_sh_l\n"); }
		int64 value2 = stack.pop_back<int64>();
		int64 value1 = stack.pop_back<int64>();
		stack.emplace_back(int64{ value1 << (value2 & 0x3F) });
	}
	void operator () (instr::i_sh_r) {
		if(info) { tabs(); print::out("i_sh_r\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		stack.emplace_back(int32{ value1 >> (value2 & 0x1F) });
	}
	void operator () (instr::l_sh_r) {
		if(info) { tabs(); print::out("l_sh_r\n"); }
		int32 value2 = stack.pop_back<int64>();
		int64 value1 = stack.pop_back<int64>();
		stack.emplace_back(int64{ value1 >> (value2 & 0x3F) });
	}
	void operator () (instr::i_u_sh_r) {
		if(info) { tabs(); print::out("i_u_sh_r\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		stack.emplace_back(int32 {
			int32(uint32(value1) >> (value2 & 0x1F))
		});
	}
	void operator () (instr::l_u_sh_r) {
		if(info) { tabs(); print::out("l_ush_r\n"); }
		int32 value2 = stack.pop_back<int32>();
		int64 value1 = stack.pop_back<int64>();
		stack.emplace_back(int64 {
			int64(uint64(value1) >> (value2 & 0x3F))
		});
	}
	void operator () (instr::i_and) {
		if(info) { tabs(); print::out("i_and "); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		int32 result = value1 & value2;
		if(info) {
			print::out(value1, " ", value2, " = ", result, "\n");
		}
		stack.emplace_back(result);
	}
	void operator () (instr::l_and) {
		if(info) { tabs(); print::out("l_and\n"); }
		int64 value2 = stack.pop_back<int64>();
		int64 value1 = stack.pop_back<int64>();
		stack.emplace_back(int64{ value1 & value2 });
	}
	void operator () (instr::i_or) {
		if(info) { tabs(); print::out("i_or "); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		int32 result = value1 | value2;
		if(info) { print::out(value1, " ", value2, " = ", result, "\n"); }
		stack.emplace_back(result);
	}
	void operator () (instr::l_or) {
		if(info) { tabs(); print::out("l_or\n"); }
		int64 value2 = stack.pop_back<int64>();
		int64 value1 = stack.pop_back<int64>();
		stack.emplace_back(int64{ value1 | value2 });
	}
	void operator () (instr::i_xor) {
		if(info) { tabs(); print::out("i_xor\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		stack.emplace_back(int32{ value1 ^ value2 });
	}
	void operator () (instr::l_xor) {
		if(info) { tabs(); print::out("l_xor\n"); }
		int64 value2 = stack.pop_back<int64>();
		int64 value1 = stack.pop_back<int64>();
		stack.emplace_back(int64{ value1 ^ value2 });
	}
	void operator () (instr::i_inc x) {
		if(info) { tabs(); print::out("i_inc ", x.index, " ", x.value, "\n"); }
		stack.get<int32>(locals_begin + x.index) += x.value;
	}
	void operator () (instr::i_to_l) {
		if(info) { tabs(); print::out("i_to_l\n"); }
		int32 value = stack.pop_back<int32>();
		stack.emplace_back((int64) value);
	}
	void operator () (instr::i_to_f) {
		if(info) { tabs(); print::out("i_to_f\n"); }
		int32 value = stack.pop_back<int32>();
		stack.emplace_back((float) value);
	}
	void operator () (instr::i_to_d) {
		if(info) { tabs(); print::out("i_to_d\n"); }
		int32 value = stack.pop_back<int32>();
		stack.emplace_back((double) value);
	}
	void operator () (instr::l_to_i) {
		if(info) { tabs(); print::out("l_to_i\n"); }
		int64 value = stack.pop_back<int64>();
		stack.emplace_back((int32) (value & 0xFFFFFFFF));
	}
	void operator () (instr::l_to_f) {
		if(info) { tabs(); print::out("l_to_f\n"); }
		int64 value = stack.pop_back<int64>();
		stack.emplace_back((float) value);
	}
	void operator () (instr::l_to_d) {
		if(info) { tabs(); print::out("l_to_d\n"); }
		int64 value = stack.pop_back<int64>();
		stack.emplace_back((double) value);
	}
	void operator () (instr::f_to_i) {
		if(info) { tabs(); print::out("f_to_i\n"); }
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
	void operator () (instr::f_to_l) {
		if(info) { tabs(); print::out("f_to_l\n"); }
		float value = stack.pop_back<float>();
		stack.emplace_back((int64) value);
	}
	void operator () (instr::f_to_d) {
		if(info) { tabs(); print::out("f_to_d\n"); }
		float value = stack.pop_back<float>();
		stack.emplace_back((double) value);
	}
	void operator () (instr::d_to_i) {
		if(info) { tabs(); print::out("d_to_i\n"); }
		double value = stack.pop_back<double>();
		stack.emplace_back((int32) value);
	}
	void operator () (instr::d_to_l) {
		if(info) { tabs(); print::out("d_to_l\n"); }
		double value = stack.pop_back<double>();
		stack.emplace_back((int64) value);
	}
	void operator () (instr::d_to_f) {
		if(info) { tabs(); print::out("d_to_f\n"); }
		double value = stack.pop_back<double>();
		stack.emplace_back((float) value);
	}
	void operator () (instr::i_to_b) {
		if(info) { tabs(); print::out("i_to_b\n"); }
		int32 value = stack.pop_back<int32>();
		stack.emplace_back((int32) (uint32) (int8) value);
	}
	void operator () (instr::i_to_c) {
		if(info) { tabs(); print::out("i_to_c\n"); }
		int32 value = stack.pop_back<int32>();
		stack.emplace_back((int32) (uint32) (uint16) (uint32) value);
	}
	void operator () (instr::i_to_s) {
		if(info) { tabs(); print::out("i_to_s\n"); }
		int32 value = stack.pop_back<int32>();
		stack.emplace_back((int32) (uint32) (int16) value);
	}
	void operator () (instr::l_cmp) {
		if(info) { tabs(); print::out("l_cmp\n"); }
		int64 value_2 = stack.pop_back<int64>();
		int64 value_1 = stack.pop_back<int64>();
		int32 result;
		if(value_1 >  value_2) result =  1;
		if(value_1 == value_2) result =  0;
		if(value_1 <  value_2) result = -1;
		stack.emplace_back(result);
	}
	void operator () (instr::f_cmp_l) {
		if(info) { tabs(); print::out("f_cmp_l\n"); }
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
	void operator () (instr::f_cmp_g) {
		if(info) { tabs(); print::out("f_cmp_g\n"); }
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
	void operator () (instr::d_cmp_l) {
		if(info) { tabs(); print::out("d_cmp_l\n"); }
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
	}
	void operator () (instr::d_cmp_g) {
		if(info) { tabs(); print::out("d_cmp_g\n"); }
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
	}
	void operator () (instr::if_eq x) {
		if(info) { tabs(); print::out("if_eq "); }
		int32 value = stack.pop_back<int32>();
		if(info) {
			print::out(value, " +", x.branch, "\n");
		}
		if(value == 0) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_ne x) {
		if(info) { tabs(); print::out("if_ne ", x.branch, "\n"); }
		int32 value = stack.pop_back<int32>();
		if(value != 0) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_lt x) {
		if(info) { tabs(); print::out("if_lt ", x.branch, "\n"); }
		int32 value = stack.pop_back<int32>();
		if(value < 0) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_ge x) {
		if(info) { tabs(); print::out("if_ge ", x.branch, "\n"); }
		int32 value = stack.pop_back<int32>();
		if(value >= 0) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_gt x) {
		if(info) { tabs(); print::out("if_gt ", x.branch, "\n"); }
		int32 value = stack.pop_back<int32>();
		if(value > 0) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_le x) {
		if(info) { tabs(); print::out("if_le ", x.branch, "\n"); }
		int32 value = stack.pop_back<int32>();
		if(value <= 0) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_i_cmp_eq x) {
		if(info) { tabs(); print::out("if_i_cmp_eq ", x.branch, "\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		if(value1 == value2) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_i_cmp_ne x) {
		if(info) { tabs(); print::out("if_i_cmp_ne ", x.branch, "\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		if(value1 != value2) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_i_cmp_lt x) {
		if(info) { tabs(); print::out("if_i_cmp_lt ", x.branch, "\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		if(value1 < value2) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_i_cmp_ge x) {
		if(info) { tabs(); print::out("if_i_cmp_ge ", x.branch, "\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		if(value1 >= value2) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_i_cmp_gt x) {
		if(info) { tabs(); print::out("if_i_cmp_gt ", x.branch, "\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		if(value1 > value2) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_i_cmp_le x) {
		if(info) { tabs(); print::out("if_i_cmp_le ", x.branch, "\n"); }
		int32 value2 = stack.pop_back<int32>();
		int32 value1 = stack.pop_back<int32>();
		if(value1 <= value2) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_a_cmp_eq x) {
		if(info) { tabs(); print::out("if_a_cmp_eq ", x.branch, "\n"); }
		reference value2 = stack.pop_back<reference>();
		reference value1 = stack.pop_back<reference>();
		if(value1.object_ptr() == value2.object_ptr()) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_a_cmp_ne x) {
		if(info) { tabs(); print::out("if_a_cmp_ne ", x.branch, "\n"); }
		reference value2 = stack.pop_back<reference>();
		reference value1 = stack.pop_back<reference>();
		if(value1.object_ptr() != value2.object_ptr()) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::go_to x) {
		if(info) { tabs(); print::out("go_to ", x.branch, "\n"); }
		next_instruction_ptr = instruction_ptr + x.branch;
	}
	void operator () (instr::jmp_sr x) {
		if(info) { tabs(); print::out("jmp_sr\n"); }
		uint32 address = (uint32) (uint64) next_instruction_ptr;
		stack.emplace_back((int32)address);
		next_instruction_ptr = instruction_ptr + x.branch;
	}
	void operator () (instr::return_sr x) {
		if(info) { tabs(); print::out("return_sr\n"); }
		uint32 address = stack.get<int32>(x.index);
		next_instruction_ptr = m.code().iterator() + address;
	}
	void operator () (instr::table_switch x) {
		if(info) { tabs(); print::out("table_switch\n"); }
		/* The index must be of type int and is popped from the operand
			stack. */
		int32 index = stack.pop_back<int32>();

		/* If index is less than low or index is greater than high, ... */
		if(index < x.low || index > x.hight) {
			/* ... then a target address is calculated by adding default to
				the address of the opcode of this tableswitch instruction */
			next_instruction_ptr = instruction_ptr + x._default;
		}
		else {
			/* Otherwise, the offset at position index - low of the jump
			table is extracted */
			uint32 position = index - x.low;
			int32 offset = x.offsets[position];
			/* The target address is calculated by adding that offset to the
			address of the opcode of this tableswitch instruction.
			Execution then continues at the target address. */
			next_instruction_ptr = instruction_ptr + offset;
		}
	}
	loop_action operator () (instr::lookup_switch x) {
		if(info) { tabs(); print::out("lookup_switch\n"); }

		int32 key = stack.pop_back<int32>();

		for(instr::match_offset mo : x.pairs) {
			/*  The key is compared against the match values. */
			if(mo.match == key) {
				/* If it is equal to one of them, then a target address is
					calculated by adding the corresponding offset to the
					address of the opcode of this lookupswitch instruction.*/
				next_instruction_ptr = instruction_ptr + mo.offset;
				return loop_action::next;
			}
		}
		/*  If the key does not match any of the match values, the target
			address is calculated by adding default to the address of the
			opcode of this lookupswitch instruction. */
		next_instruction_ptr = instruction_ptr + x._default;
		return loop_action::next;
	}
	loop_action operator () (instr::i_return) {
		if(info) { tabs(); print::out("i_return "); }

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
		if(info) { print::out(result, "\n"); }
		return loop_action::stop;
	}
	loop_action operator () (instr::l_return) {
		if(info) { tabs(); print::out("l_return\n"); }
		int64 result = stack.pop_back<int64>();
		stack.pop_back_until(locals_begin);
		stack.emplace_back(result);
		return loop_action::stop;
	}
	loop_action operator () (instr::f_return) {
		if(info) { tabs(); print::out("f_return\n"); }
		float result = stack.pop_back<float>();
		stack.pop_back_until(locals_begin);
		stack.emplace_back(result);
		return loop_action::stop;
	}
	loop_action operator () (instr::d_return) {
		if(info) { tabs(); print::out("d_return\n"); }
		double result = stack.pop_back<double>();
		stack.pop_back_until(locals_begin);
		stack.emplace_back(result);
		return loop_action::stop;
	}
	loop_action operator () (instr::a_return) {
		if(info) { tabs(); print::out("a_return\n"); }
		reference result = stack.pop_back<reference>();
		stack.pop_back_until(locals_begin);
		stack.emplace_back(move(result));
		return loop_action::stop;
	}
	loop_action operator () (instr::_return) {
		if(info) { tabs(); print::out("return\n"); }
		stack.pop_back_until(locals_begin);
		return loop_action::stop;
	}
	loop_action operator () (instr::get_static x) {
		namespace cc = class_file::constant;
		if(info) {
			tabs(); print::out("get_static ");
			cc::field_ref field_ref = c.field_ref_constant(x.index);
			cc::_class _class = c.class_constant(field_ref.class_index);
			cc::utf8 class_name = c.utf8_constant(_class.name_index);
			cc::name_and_type nat {
				c.name_and_type_constant(field_ref.name_and_type_index)
			};
			cc::utf8 name = c.utf8_constant(nat.name_index);
			print::out(class_name, ".", name, " @", stack.size(), "\n");
		}

		expected<class_and_declared_static_field_index, reference>
		possible_class_and_field_index
			= c.try_get_static_field_index(x.index);
		
		if(possible_class_and_field_index.is_unexpected()) {
			return handle_thrown(
				move(possible_class_and_field_index.get_unexpected())
			);
		}

		class_and_declared_static_field_index class_and_field_index
			= possible_class_and_field_index.get_expected();
		
		class_and_field_index._class.view(
			class_and_field_index.field_index,
			[](auto& field_value) {
				stack.emplace_back(field_value);
			}
		);
		return loop_action::next;
	}
	loop_action operator () (instr::put_static x) {
		namespace cc = class_file::constant;
		if(info) {
			tabs(); print::out("put_static ");
			cc::field_ref field_ref = c.field_ref_constant(x.index);
			cc::name_and_type nat {
				c.name_and_type_constant(field_ref.name_and_type_index)
			};
			cc::utf8 name = c.utf8_constant(nat.name_index);
			print::out(name, "\n");
		}
		expected<class_and_declared_static_field_index, reference>
		possible_class_and_field_index
			= c.try_get_static_field_index(x.index);
		
		if(possible_class_and_field_index.is_unexpected()) {
			return handle_thrown(
				move(possible_class_and_field_index.get_unexpected())
			);
		}

		class_and_declared_static_field_index class_and_field_index
			= possible_class_and_field_index.get_expected();

		class_and_field_index._class.view(
			class_and_field_index.field_index,
			[]<typename FieldType>(FieldType& field_value) {
				field_value = stack.pop_back<FieldType>();
			}
		);
		return loop_action::next;
	}
	loop_action operator () (instr::get_field x) {
		namespace cc = class_file::constant;
		if(info) {
			tabs(); print::out("get_field ");
			auto field_ref = c.field_ref_constant(x.index);
			cc::name_and_type nat = c.name_and_type_constant(
				field_ref.name_and_type_index
			);
			cc::_class class_ = c.class_constant(field_ref.class_index);
			cc::utf8 class_name = c.utf8_constant(class_.name_index);
			cc::utf8 field_name = c.utf8_constant(nat.name_index);
			print::out(class_name, ".", field_name, "\n");
		}

		reference ref = stack.pop_back<reference>();

		if(ref.is_null()) {
			return handle_thrown(try_create_null_pointer_exception().get());
		}

		expected<instance_field_index_and_stack_size, reference> possible_field
			= c.try_get_resolved_instance_field_index(x.index);

		if(possible_field.is_unexpected()) {
			return handle_thrown(
				move(possible_field.get_unexpected())
			);
		}

		instance_field_index_and_stack_size field_index_and_stack_size
			= possible_field.get_expected();

		ref->view(
			field_index_and_stack_size.field_index,
			[&](auto& field_value) {
				stack.emplace_back(field_value);
			}
		);
		return loop_action::next;
	}
	loop_action operator () (instr::put_field x) {
		namespace cc = class_file::constant;
		if(info) {
			tabs(); print::out("put_field ");
			cc::field_ref field_ref = c.field_ref_constant(x.index);
			cc::name_and_type nat = c.name_and_type_constant(
				field_ref.name_and_type_index
			);
			cc::_class class_ = c.class_constant(field_ref.class_index);
			cc::utf8 class_name = c.utf8_constant(class_.name_index);
			cc::utf8 name = c.utf8_constant(nat.name_index);
			print::out(class_name, ".", name, "\n");
		}

		expected<instance_field_index_and_stack_size, reference>
		possible_field_index_and_stack_size
			= c.try_get_resolved_instance_field_index(x.index);

		if(possible_field_index_and_stack_size.is_unexpected()) {
			return handle_thrown(
				move(possible_field_index_and_stack_size.get_unexpected())
			);
		}

		instance_field_index_and_stack_size field_index_and_stack_size
			= possible_field_index_and_stack_size.get_expected();

		reference ref = move(stack.get<reference>(
			stack.size() - 1 - field_index_and_stack_size.stack_size
		));

		if(ref.is_null()) {
			return handle_thrown(try_create_null_pointer_exception().get());
		}

		ref->view(
			field_index_and_stack_size.field_index,
			[&]<typename FieldType>(FieldType& field_value) {
				field_value = stack.pop_back<FieldType>();
			}
		);

		stack.pop_back<reference>();
		return loop_action::next;
	}
	loop_action operator () (instr::invoke_virtual x) {
		_class& d = c;

		if(info) {
			class_file::constant::method_ref method_ref
				= c.method_ref_constant(x.index);
			class_file::constant::name_and_type nat =
					c.name_and_type_constant(method_ref.name_and_type_index);
			class_file::constant::utf8 desc
				= c.utf8_constant(nat.descriptor_index);
			class_file::constant::utf8 name
				= c.utf8_constant(nat.name_index);

			class_file::constant::_class c
				= d.class_constant(method_ref.class_index);
			class_file::constant::utf8 c_name = d.utf8_constant(c.name_index);
			tabs(); print::out(
				"invoke_virtual ", c_name, ".", name, desc, "\n"
			);
		}
		optional<reference> possible_throwable
			= ::try_invoke_virtual(d, x.index);

		if(possible_throwable.has_value()) {
			return handle_thrown(move(possible_throwable.get()));
		}
		return loop_action::next;
	}
	loop_action operator () (
		class_file::attribute::code::instruction::invoke_special x
	) {
		_class& d = c;

		if(info) {
			d.view_method_or_interface_method_constant(
				x.index, [&](auto method_ref) {
					class_file::constant::name_and_type nat
						= d.name_and_type_constant(
							method_ref.name_and_type_index
						);
					class_file::constant::utf8 desc
						= d.utf8_constant(nat.descriptor_index);
					tabs(); print::out("invoke_special ");
					class_file::constant::_class c
						= d.class_constant(method_ref.class_index);
					class_file::constant::utf8 c_name
						= d.utf8_constant(c.name_index);
					class_file::constant::utf8
						name = d.utf8_constant(nat.name_index);
					print::out(c_name, ".", name, desc, "\n");
				}
			);
		}

		optional<reference> possible_throwable
			= ::try_invoke_special(c, x.index);

		if(possible_throwable.has_value()) {
			return handle_thrown(move(possible_throwable.get()));
		}
		return loop_action::next;
	}
	loop_action operator () (instr::invoke_static x) {
		_class& d = c;

		if(info) {
			d.view_method_or_interface_method_constant(
				x.index,
				[&](auto method_ref) {
					class_file::constant::name_and_type nat
						= d[method_ref.name_and_type_index];

					class_file::constant::utf8 desc = d[nat.descriptor_index];

					tabs(); print::out("invoke_static ");

					class_file::constant::_class c = d[method_ref.class_index];
					class_file::constant::utf8 c_name = d[c.name_index];

					class_file::constant::utf8 name = d[nat.name_index];

					print::out(c_name, ".", name, desc, "\n");
				}
			);

		}

		optional<reference> possible_throwable
			= ::try_invoke_static(d, x.index);

		if(possible_throwable.has_value()) {
			return handle_thrown(move(possible_throwable.get()));
		}
		return loop_action::next;
	}
	loop_action operator () (instr::invoke_interface x) {
		_class& d = c;

		if(info) {
			class_file::constant::interface_method_ref method_ref
				= d.interface_method_ref_constant(x.index);
			class_file::constant::name_and_type nat
				= d.name_and_type_constant(method_ref.name_and_type_index);

			auto name = d.utf8_constant(nat.name_index);
			auto desc = d.utf8_constant(nat.descriptor_index);
			class_file::constant::_class class_info
				= d.class_constant(method_ref.class_index);
			auto class_name = d.utf8_constant(class_info.name_index);
			tabs(); print::out(
				"invoke_interface ", class_name, ".", name, desc, "\n"
			);
		}

		optional<reference> possible_throwable
			= ::try_invoke_interface(d, x.index);

		if(possible_throwable.has_value()) {
			return handle_thrown(move(possible_throwable.get()));
		}
		return loop_action::next;
	}
	loop_action operator () (instr::invoke_dynamic x) {
		optional<reference> possible_throwable
			= ::try_invoke_dynamic(x.index, c);

		if(possible_throwable.has_value()) {
			return handle_thrown(move(possible_throwable.get()));
		}
		return loop_action::next;
	}
	loop_action operator () (instr::_new x) {
		if(info) {
			tabs(); print::out("new ");
			class_file::constant::utf8 name = c.utf8_constant(
				c.class_constant(x.index).name_index
			);
			print::out(name, " @", stack.size(), "\n");
		}
		expected<_class&, reference> possible_c0
			= c.try_get_resolved_class(x.index);
		
		if(possible_c0.is_unexpected()) {
			return handle_thrown(move(possible_c0.get_unexpected()));
		}

		_class& c0 = possible_c0.get_expected();
		expected<reference, reference> possible_ref = try_create_object(c0);
		if(possible_ref.is_unexpected()) {
			return handle_thrown(move(possible_ref.get_unexpected()));
		}
		reference ref = move(possible_ref.get_expected());
		stack.emplace_back(move(ref));
		return loop_action::next;
	}
	loop_action operator () (instr::new_array x) {
		optional<reference> possible_throwable
			= ::try_new_array(/* c, */ x.type);
		if(possible_throwable.has_value()) {
			return handle_thrown(move(possible_throwable.get()));
		}
		return loop_action::next;
	}
	loop_action operator () (instr::a_new_array x) {
		expected<_class&, reference> possible_element_class
			= c.try_get_resolved_class(x.index);
		
		if(possible_element_class.is_unexpected()) {
			return handle_thrown(move(possible_element_class.get_unexpected()));
		}

		_class& element_class = possible_element_class.get_expected();

		if(info) {
			tabs(); print::out("a_new_array ");
			class_file::constant::utf8 name = element_class.name();
			print::out(name, "\n");
		}

		int32 count = stack.pop_back<int32>();
		expected<reference, reference> possible_ref
			= try_create_array_of(element_class, count);

		if(possible_ref.is_unexpected()) {
			return handle_thrown(move(possible_ref.get_unexpected()));
		}

		reference ref = move(possible_ref.get_expected());

		stack.emplace_back(move(ref));
		return loop_action::next;
	}
	loop_action operator () (instr::array_length) {
		if(info) { tabs(); print::out("array_length\n"); }
		reference ref = stack.pop_back<reference>();
		if(ref.is_null()) {
			expected<reference, reference> possible_npe
				= try_create_null_pointer_exception();
			return handle_thrown(move(possible_npe.get()));
		}
		stack.emplace_back(int32{ ::array_length(ref) });
		return loop_action::next;
	}
	loop_action operator () (instr::a_throw) {
		if(info) { tabs(); print::out("a_throw\n"); }

		reference ref = move(stack.pop_back<reference>());
		if(ref.is_null()) {
			expected<reference, reference> possible_npe
				= try_create_null_pointer_exception();
			return handle_thrown(move(possible_npe.get()));
		}
		return handle_thrown(move(ref));
	}
	loop_action operator () (instr::check_cast x) {
		optional<reference> possible_throwable = ::try_check_cast(c, x.index);
		if(possible_throwable.has_value()) {
			return handle_thrown(move(possible_throwable.get()));
		}
		return loop_action::next;
	}
	loop_action operator () (instr::instance_of x) {
		optional<reference> possible_throwable
			= ::try_check_instance_of(c, x.index);
		if(possible_throwable.has_value()) {
			return handle_thrown(move(possible_throwable.get()));
		}
		return loop_action::next;
	}
	loop_action operator () (instr::monitor_enter) {
		reference ref = stack.pop_back<reference>();
		if(ref.is_null()) {
			expected<reference, reference> possible_npe
				= try_create_null_pointer_exception();
			return handle_thrown(move(possible_npe.get()));
		}
		ref->lock();
		return loop_action::next;
	}
	loop_action operator () (instr::monitor_exit) {
		reference ref = stack.pop_back<reference>();
		if(ref.is_null()) {
			expected<reference, reference> possible_npe
				= try_create_null_pointer_exception();
			return handle_thrown(move(possible_npe.get()));
		}
		ref->unlock();
		return loop_action::next;
	}
	void operator () (instr::if_null x) {
		if(info) { tabs(); print::out("if_null ", x.branch, "\n"); }
		reference ref = stack.pop_back<reference>();
		if(ref.is_null()) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::if_non_null x) {
		if(info) { tabs(); print::out("if_non_null ", x.branch, "\n"); }
		reference ref = stack.pop_back<reference>();
		if(!ref.is_null()) {
			next_instruction_ptr = instruction_ptr + x.branch;
		}
	}
	void operator () (instr::go_to_w x) {
		if(info) { tabs(); print::out("go_to_w ", x.branch, "\n"); }
		next_instruction_ptr = instruction_ptr + x.branch;
	}
	void operator () (instr::jmp_sr_w x) {
		if(info) { tabs(); print::out("jmp_sr_w\n"); }
		uint32 address = (uint32) (uint64) next_instruction_ptr;
		stack.emplace_back((int32)address);
		next_instruction_ptr = instruction_ptr + x.branch;
	}
	void operator () (instr::unknown x) {
		if(info) {
			tabs();
			print::err("unknown instruction ", x.code, "\n");
		}
		posix::abort();
	}
	void operator () (auto x) {
		if(info) {
			tabs();
			print::err("unimplemented instruction ", decltype(x)::code, "\n");
		}
		posix::abort();
	}

};