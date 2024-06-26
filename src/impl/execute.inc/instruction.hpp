#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"

#include "decl/class.hpp"
#include "decl/classes.hpp"
#include "decl/method.hpp"
#include "decl/array.hpp"
#include "decl/reference.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/lib/java/lang/index_out_of_bounds_exception.hpp"
#include "decl/lib/java/lang/throwable.hpp"

#include "./ldc.hpp"
#include "./check_cast.hpp"
#include "./instance_of.hpp"
#include "./new_array.hpp"

#include <class_file/attribute/code/instruction.hpp>
#include <class_file/attribute/code/reader.hpp>

#include <posix/math.hpp>

#include <loop_action.hpp>

namespace instr = class_file::attribute::code::instruction;

struct execute_instruction {
	method& m;
	c& c;
	const uint8* const instructions_beginning_ptr;
	const uint8* instruction_ptr;
	const uint8* next_instruction_ptr;

	const nuint locals_begin;
	const nuint stack_begin;
	reference thrown;

	[[nodiscard]] loop_action handle_thrown(reference thrown) {
		if(info) {
			tabs();
			print::out("handling ", thrown.c().name(), "\n");
		}
		::c& thrown_class = thrown->c();

		auto& exception_handlers = m.exception_handlers();

		for(auto handler : exception_handlers) {
			uint32 pc = instruction_ptr - m.code().iterator();
			bool in_range = pc >= handler.start_pc && pc < handler.end_pc;
			if(!in_range) {
				continue;
			}

			expected<::c&, reference> possible_catch_class
				= c.try_get_resolved_class(handler.catch_type);
			
			if(possible_catch_class.is_unexpected()) {
				reference new_throwable_ref
					= possible_catch_class.move_unexpected();
				j::throwable& new_throwable = (j::throwable&)
					new_throwable_ref.object();

				new_throwable.init_cause(move(thrown));

				stack.erase_back_until(locals_begin);
				this->thrown = move(new_throwable_ref);
				return loop_action::stop;
			}

			::c& catch_class = possible_catch_class.get_expected();

			bool same = &thrown_class == &catch_class;
			bool subclass = thrown_class.is_sub_of(catch_class);

			if(!(same || subclass)) {
				continue;
			}

			next_instruction_ptr = m.code().iterator() + handler.handler_pc;

			stack.erase_back_until(stack_begin);
			stack.emplace_back(move(thrown));
			return loop_action::next;
		}
		if(info) {
			tabs();
			print::out("didn't find any exception handlers\n");
		}
		stack.erase_back_until(locals_begin);
		this->thrown = move(thrown);
		return loop_action::stop;
	}

	template<typename E, typename Handler>
	loop_action view_array(Handler&& handler) {
		int32 element_index = stack.pop_back<int32>();
		reference array_ref = stack.pop_back<reference>();
		if(array_ref.is_null()) {
			return handle_thrown(try_create_null_pointer_exception().get());
		}
		int32 len = ::array_length(array_ref);
		if(element_index < 0 || element_index >= len) {
			::c& exception_c = classes.load_non_array_class(
				u8"java/lang/ArrayIndexOutOfBoundsException"sv,
				(j::c_loader*) c.defining_loader().object_ptr()
			);
			instance_method& m = exception_c.declared_instance_methods().find(
				u8"<init>"sv, u8"(I)V"sv
			);
			return handle_thrown(
				try_create_object(m, element_index).get()
			);
		}
		E* ptr = array_data<E>(array_ref);
		handler(array_ref, element_index, ptr[element_index]);
		return loop_action::next;
	};

	template<typename Type>
	void print_value(Type& v) {
		if constexpr(!same_as<Type, reference>) {
			print::out(v);
		}
		else {
			if(!v.is_null()) {
				print::out(v.c().name());
				print::out(" @");
				print::out.hex((uint64) v.object_ptr());
			}
			else {
				print::out("null");
			}
		}
	}

	template<typename Type>
	void print_value_at(nuint stack_pos, Type& v) {
		print::out("stack[", stack_pos, "]{ ");
		print_value(v);
		print::out(" }");
	}

	void operator () (instr::nop) {
		if(info) { tabs(); print::out("nop\n"); }
	}

	template<char Prefix, typename Type>
	void constant(Type&& value) {
		if(info) {
			tabs();
			print::out(Prefix, "_const ");
			print_value(value);
			print::out(": stack[", stack.size(), "] = ");
			print_value(value);
			print::out("\n");
		}
		stack.emplace_back(move(value));
	}
	void operator () (instr::a_const_null) { constant<'a'>(reference{}); }
	void operator () (instr::i_const_m1) { constant<'i'>(int32{ -1 }); }
	void operator () (instr::i_const_0) { constant<'i'>(int32{ 0 }); }
	void operator () (instr::i_const_1) { constant<'i'>(int32{ 1 }); }
	void operator () (instr::i_const_2) { constant<'i'>(int32{ 2 }); }
	void operator () (instr::i_const_3) { constant<'i'>(int32{ 3 }); }
	void operator () (instr::i_const_4) { constant<'i'>(int32{ 4 }); }
	void operator () (instr::i_const_5) { constant<'i'>(int32{ 5 }); }
	void operator () (instr::l_const_0) { constant<'l'>(int64{ 0 }); }
	void operator () (instr::l_const_1) { constant<'l'>(int64{ 1 }); }
	void operator () (instr::f_const_0) { constant<'f'>(float{ 0.0F }); }
	void operator () (instr::f_const_1) { constant<'f'>(float{ 1.0F }); }
	void operator () (instr::f_const_2) { constant<'f'>(float{ 2.0F }); }
	void operator () (instr::d_const_0) { constant<'d'>(double{ 0.0 }); }
	void operator () (instr::d_const_1) { constant<'d'>(double{ 1.0 }); }

	template<typename Type>
	void push(span<const char> prefix, Type value) {
		if(info) {
			tabs();
			print::out(
				prefix, "_push: stack[", stack.size(), "] = ", value, "\n"
			);
		}
		stack.emplace_back(int32{ value });
	}
	void operator () (instr::bi_push x) { push("bi"sv, x.value); }
	void operator () (instr::si_push x) { push("si"sv, x.value); }

	loop_action operator () (
		same_as_any<
			class_file::attribute::code::instruction::ldc,
			class_file::attribute::code::instruction::ldc_w
		> auto x
	) {
		optional<reference> possible_throwable
			= ::try_ldc(x.constant_index, c);
		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.get());
		}
		return loop_action::next;
	}
	void operator () (class_file::attribute::code::instruction::ldc_2_w x) {
		::ldc_2_w(x.constant_index, c);
	}
	template<typename Type>
	void load(nuint offset, auto&&... to_print) {
		Type value = stack.get<Type>(locals_begin + offset);
		if(info) {
			tabs();
			print::out(move(to_print)..., ": ");
			print::out("stack[", stack.size(), "] = ");
			print::out("locals[", locals_begin + offset, "]{ ");
			print_value(value);
			print::out(" }\n");
		}
		stack.emplace_back(move(value));
	}
	template<typename Type, char Prefix>
	void load(nuint offset) {
		load<Type>(offset, Prefix, "_load ", offset);
	}
	void operator () (instr::i_load x) { load<int32, 'i'>(x.index); }
	void operator () (instr::l_load x) { load<int64, 'l'>(x.index); }
	void operator () (instr::f_load x) { load<float, 'f'>(x.index); }
	void operator () (instr::d_load x) { load<double, 'd'>(x.index); }
	void operator () (instr::a_load x) { load<reference, 'a'>(x.index); }

	template<typename Type, char Prefix, nuint Offset>
	void load() {
		load<Type>(Offset, Prefix, "_load_", Offset);
	}
	void operator () (instr::i_load_0) { load<int32, 'i', 0>(); }
	void operator () (instr::i_load_1) { load<int32, 'i', 1>(); }
	void operator () (instr::i_load_2) { load<int32, 'i', 2>(); }
	void operator () (instr::i_load_3) { load<int32, 'i', 3>(); }
	void operator () (instr::l_load_0) { load<int64, 'l', 0>(); }
	void operator () (instr::l_load_1) { load<int64, 'l', 1>(); }
	void operator () (instr::l_load_2) { load<int64, 'l', 2>(); }
	void operator () (instr::l_load_3) { load<int64, 'l', 3>(); }
	void operator () (instr::f_load_0) { load<float, 'f', 0>(); }
	void operator () (instr::f_load_1) { load<float, 'f', 1>(); }
	void operator () (instr::f_load_2) { load<float, 'f', 2>(); }
	void operator () (instr::f_load_3) { load<float, 'f', 3>(); }
	void operator () (instr::d_load_0) { load<double, 'd', 0>(); }
	void operator () (instr::d_load_1) { load<double, 'd', 1>(); }
	void operator () (instr::d_load_2) { load<double, 'd', 2>(); }
	void operator () (instr::d_load_3) { load<double, 'd', 3>(); }
	void operator () (instr::a_load_0) { load<reference, 'a', 0>(); }
	void operator () (instr::a_load_1) { load<reference, 'a', 1>(); }
	void operator () (instr::a_load_2) { load<reference, 'a', 2>(); }
	void operator () (instr::a_load_3) { load<reference, 'a', 3>(); }

	template<typename Type, char Prefix>
	loop_action array_load() {
		return view_array<Type>([&](reference& ref, int32 index, Type& v) {
			if(info) {
				tabs();
				print::out(Prefix, "_a_load: stack[", stack.size(), "] = ");
				print_value_at(stack.size(), ref);
				print::out("[");
				print_value_at(stack.size() + 1, index);
				print::out("]{ ");
				print_value(v);
				print::out(" }\n");
			}
			stack.emplace_back(v);
		});
	}
	loop_action operator () (instr::i_a_load) {
		return array_load<int32, 'i'>();
	}
	loop_action operator () (instr::l_a_load) {
		return array_load<int64, 'l'>();
	}
	loop_action operator () (instr::f_a_load) {
		return array_load<float, 'f'>();
	}
	loop_action operator () (instr::d_a_load) {
		return array_load<double, 'd'>();
	}
	loop_action operator () (instr::a_a_load) {
		return array_load<reference, 'a'>();
	}
	loop_action operator () (instr::b_a_load) {
		return array_load<int8, 'b'>();
	}
	loop_action operator () (instr::c_a_load) {
		return array_load<uint16, 'c'>();
	}
	loop_action operator () (instr::s_a_load) {
		return array_load<int16, 's'>();
	}

	template<typename Type>
	void store(nuint offset, auto&&... to_print) {
		Type value = stack.pop_back<Type>();
		if(info) {
			tabs(); print::out(move(to_print)..., ": ");
			print::out("locals[", locals_begin + offset, "] = ");
			print_value_at(stack.size(), value);
			print::out("\n");
		}
		stack.emplace_at(locals_begin + offset, move(value));
	}

	template<typename Type, char Prefix>
	void store(nuint offset) {
		store<Type>(offset, Prefix, "_store ", offset);
	}
	void operator () (instr::i_store x) { store<int32, 'i'>(x.index); }
	void operator () (instr::l_store x) { store<int64, 'l'>(x.index); }
	void operator () (instr::f_store x) { store<float, 'f'>(x.index); }
	void operator () (instr::d_store x) { store<double, 'd'>(x.index); }
	void operator () (instr::a_store x) { store<reference, 'a'>(x.index); }

	template<typename Type, char Prefix, nuint Offset>
	void store() {
		store<Type>(Offset, Prefix, "_store_", Offset);
	}
	void operator () (instr::i_store_0) { store<int32, 'i', 0>(); }
	void operator () (instr::i_store_1) { store<int32, 'i', 1>(); }
	void operator () (instr::i_store_2) { store<int32, 'i', 2>(); }
	void operator () (instr::i_store_3) { store<int32, 'i', 3>(); }
	void operator () (instr::l_store_0) { store<int64, 'l', 0>(); }
	void operator () (instr::l_store_1) { store<int64, 'l', 1>(); }
	void operator () (instr::l_store_2) { store<int64, 'l', 2>(); }
	void operator () (instr::l_store_3) { store<int64, 'l', 3>(); }
	void operator () (instr::f_store_0) { store<float, 'f', 0>(); }
	void operator () (instr::f_store_1) { store<float, 'f', 1>(); }
	void operator () (instr::f_store_2) { store<float, 'f', 2>(); }
	void operator () (instr::f_store_3) { store<float, 'f', 3>(); }
	void operator () (instr::d_store_0) { store<double, 'd', 0>(); }
	void operator () (instr::d_store_1) { store<double, 'd', 1>(); }
	void operator () (instr::d_store_2) { store<double, 'd', 2>(); }
	void operator () (instr::d_store_3) { store<double, 'd', 3>(); }
	void operator () (instr::a_store_0) { store<reference, 'a', 0>(); }
	void operator () (instr::a_store_1) { store<reference, 'a', 1>(); }
	void operator () (instr::a_store_2) { store<reference, 'a', 2>(); }
	void operator () (instr::a_store_3) { store<reference, 'a', 3>(); }

	template<typename Type, char Prefix>
	loop_action array_store() {
		Type value = stack.pop_back<Type>();
		return view_array<Type>([&](reference& ref, int32 index, Type& v) {
			if(info) {
				tabs(); print::out(Prefix, "_a_store: ");
				print_value_at(stack.size(), ref);
				print::out("[");
				print_value_at(stack.size() + 1, index);
				print::out("]{ "); print_value(v); print::out(" } = ");
				print_value_at(stack.size() + 2, value);
				print::out("\n");
			}
			v = value;
		});
	}
	loop_action operator () (instr::i_a_store) {
		return array_store<int32, 'i'>();
	}
	loop_action operator () (instr::l_a_store) {
		return array_store<int64, 'l'>();
	}
	loop_action operator () (instr::f_a_store) {
		return array_store<float, 'f'>();
	}
	loop_action operator () (instr::d_a_store) {
		return array_store<double, 'd'>();
	}
	loop_action operator () (instr::a_a_store) {
		return array_store<reference, 'a'>();
	}
	loop_action operator () (instr::b_a_store) {
		return array_store<int8, 'b'>();
	}
	loop_action operator () (instr::c_a_store) {
		return array_store<uint16, 'c'>();
	}
	loop_action operator () (instr::s_a_store) {
		return array_store<int16, 's'>();
	}
	void operator () (instr::pop) {
		if(info) { tabs(); print::out("pop\n"); }
		stack.erase_back();
	}
	void operator () (instr::pop_2) {
		if(info) { tabs(); print::out("pop_2\n"); }
		stack.erase_back(2);
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
	template<
		typename TypeA,
		typename TypeB = TypeA,
		typename TypeR = TypeA
	>
	void binary(auto&& op_name, auto&& name, auto op) {
		TypeB value2 = stack.pop_back<TypeB>();
		TypeA value1 = stack.pop_back<TypeA>();
		if(info) {
			tabs();
			print::out(
				name, ": ",
				"stack[", stack.size(), "] = "
			);
			print_value_at(stack.size() + 1, value1);
			print::out(" ", op_name, " ");
			print_value_at(stack.size(), value2);
			print::out("\n");
		}
		stack.emplace_back((TypeR)op(value1, value2));
	}
	template<typename Type, char Prefix>
	void add() { binary<Type>(
		"+", array{ Prefix, '_', 'a', 'd', 'd'},
		[](Type a, Type b){ return a + b; }
	);}
	void operator () (instr::i_add) { add<int32,  'i'>(); }
	void operator () (instr::l_add) { add<int64,  'l'>(); }
	void operator () (instr::f_add) { add<float,  'f'>(); }
	void operator () (instr::d_add) { add<double, 'd'>(); }

	template<typename Type, char Prefix>
	void sub() { binary<Type>(
		"-", array{ Prefix, '_', 's', 'u', 'b' },
		[](Type a, Type b){ return a - b; }
	);}
	void operator () (instr::i_sub) { sub<int32,  'i'>(); }
	void operator () (instr::l_sub) { sub<int64,  'l'>(); }
	void operator () (instr::f_sub) { sub<float,  'f'>(); }
	void operator () (instr::d_sub) { sub<double, 'd'>(); }

	template<typename Type, char Prefix>
	void mul() { binary<Type>(
		"*", array{ Prefix, '_', 'm', 'u', 'l' },
		[](Type a, Type b){ return a * b; }
	);}
	void operator () (instr::i_mul) { mul<int32,  'i'>(); }
	void operator () (instr::l_mul) { mul<int64,  'l'>(); }
	void operator () (instr::f_mul) { mul<float,  'f'>(); }
	void operator () (instr::d_mul) { mul<double, 'd'>(); }

	template<typename Type, char Prefix>
	void div() { binary<Type>(
		"/", array{ Prefix, '_', 'd', 'i', 'v' },
		[](Type a, Type b){ return a / b; }
	);}
	void operator () (instr::i_div) { div<int32,  'i'>(); }
	void operator () (instr::l_div) { div<int64,  'l'>(); }
	void operator () (instr::f_div) { div<float,  'f'>(); }
	void operator () (instr::d_div) { div<double, 'd'>(); }

	template<typename Type, char Prefix>
	void rem() {
		binary<Type>(
		"%", array{ Prefix, '_', 'r', 'e', 'm' },
		[](Type a, Type b){ return a - a/b*b; }
	);}
	void operator () (instr::i_rem) { rem<int32,  'i'>(); }
	void operator () (instr::l_rem) { rem<int64,  'l'>(); }
	void operator () (instr::f_rem) { rem<float,  'f'>(); } // TODO spec
	void operator () (instr::d_rem) { rem<double, 'd'>(); } // TODO spec

	template<typename Type, typename TypeR = Type>
	void unary(auto&& op_name, auto&& name, auto op) {
		Type value = stack.pop_back<Type>();
		if(info) {
			tabs();
			print::out(name, ": ");
			print_value_at(stack.size(), value);
			print::out(" = ", op_name);
			print_value_at(stack.size(), value);
			print::out("\n");
		}
		stack.emplace_back((TypeR)op(value));
	}

	template<typename Type, char Prefix>
	void neg() { unary<Type>(
		"-", array{ Prefix, '_', 'n', 'e', 'g' },
		[](Type v){ return -v; }
	);}
	void operator () (instr::i_neg) { neg<int32,  'i'>(); }
	void operator () (instr::l_neg) { neg<int64,  'l'>(); }
	void operator () (instr::f_neg) { neg<float,  'f'>(); }
	void operator () (instr::d_neg) { neg<double, 'd'>(); }

	void operator () (instr::i_sh_l) { binary<int32>(
		"<<", "i_sh_l",
		[](int32 v, int32 sh){ return v << (sh & 0x1F); }
	);}
	void operator () (instr::l_sh_l) { binary<int64, int32>(
		"<<", "l_sh_l",
		[](int64 v, int32 sh){ return v << (sh & 0x3F); }
	);}
	void operator () (instr::i_sh_r) { binary<int32>(
		">>", "i_sh_r",
		[](int32 v, int32 sh){ return v >> (sh & 0x1F); }
	);}
	void operator () (instr::l_sh_r) { binary<int64, int32>(
		">>", "l_sh_r",
		[](int64 v, int32 sh){ return v >> (sh & 0x3F); }
	);}
	void operator () (instr::i_u_sh_r) { binary<int32>(
		">>>", "i_u_sh_r",
		[](int32 v, int32 sh){ return uint32(v) >> (sh & 0x1F); }
	);}
	void operator () (instr::l_u_sh_r) { binary<int64, int32>(
		">>>", "l_u_sh_r",
		[](int64 v, int32 sh){ return uint64(v) >> (sh & 0x3F); }
	);}

	template<typename Type, char Prefix>
	void bit_and() { binary<Type>(
		"&", array{Prefix, '_', 'a', 'n', 'd'},
		[](Type a, Type b){ return a & b; }
	);}
	void operator () (instr::i_and) { bit_and<int32, 'i'>(); }
	void operator () (instr::l_and) { bit_and<int64, 'l'>(); }

	template<typename Type, char Prefix>
	void bit_or() { binary<Type>(
		"|", array{Prefix, '_', 'o', 'r'},
		[](Type a, Type b){ return a | b; }
	);}
	void operator () (instr::i_or) { bit_or<int32, 'i'>(); }
	void operator () (instr::l_or) { bit_or<int64, 'l'>(); }

	template<typename Type, char Prefix>
	void bit_xor() { binary<Type>(
		"^", array{Prefix, '_', 'x', 'o', 'r'},
		[](Type a, Type b){ return a ^ b; }
	);}
	void operator () (instr::i_xor) { bit_xor<int32, 'i'>(); }
	void operator () (instr::l_xor) { bit_xor<int64, 'l'>(); }

	void operator () (instr::i_inc x) {
		int32& value = stack.get<int32>(locals_begin + x.index);
		if(info) {
			tabs();
			print::out("i_inc: locals[", locals_begin + x.index, "]{ ");
			print_value(value);
			print::out(" } += ");
			print_value(x.value);
			print::out("\n");
		}
		value += x.value;
	}

	template<
		char Prefix, char Posfix,
		typename From, typename To,
		typename Op = decltype([](From v){ return (To) v; })
	>
	void cast(Op op = {}) { unary<From, To>(
		array{'(', Posfix, ')'},
		array{ Prefix, '_', 't', 'o', '_', Posfix },
		op
	);}
	void operator () (instr::i_to_l) { cast<'i', 'l', int32, int64>();  }
	void operator () (instr::i_to_f) { cast<'i', 'f', int32, float>();  }
	void operator () (instr::i_to_d) { cast<'i', 'd', int32, double>(); }
	void operator () (instr::l_to_i) { cast<'l', 'i', int64, int32>();  }
	void operator () (instr::l_to_f) { cast<'l', 'f', int64, float>();  }
	void operator () (instr::l_to_d) { cast<'l', 'd', int64, double>(); }
	void operator () (instr::f_to_i) { cast<'f', 'i', float, int32>(
		[](float v) { return posix::is_nan(v) ? (int32)0 : (int32)v; }
	);}
	void operator () (instr::f_to_l) { cast<'f', 'l', float, int64>(
		[](float v) { return posix::is_nan(v) ? (int64)0 : (int64)v; }
	);}
	void operator () (instr::f_to_d) { cast<'f', 'd', float, double>(); }
	void operator () (instr::d_to_i) { cast<'d', 'i', double, int32>(
		[](double v) { return posix::is_nan(v) ? (int32)0 : (int32)v; }
	);}
	void operator () (instr::d_to_l) { cast<'d', 'l', double, int64>(
		[](double v) { return posix::is_nan(v) ? (int64)0 : (int64)v; }
	);}
	void operator () (instr::d_to_f) { cast<'d', 'f', double, float>(); }
	void operator () (instr::i_to_b) { cast<'i', 'b', int32, int8>();   }
	void operator () (instr::i_to_c) { cast<'i', 'c', int32, uint16>(); }
	void operator () (instr::i_to_s) { cast<'i', 's', int32, int16>();  }

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
		if(info) {
			tabs(); print::out("if_eq +", x.branch);
		}
		int32 value = stack.pop_back<int32>();
		if(info) {
			print::out(", ", value, " == 0 ? ", value == 0, "\n");
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

	void operator () (instr::table_switch::reader<const uint8*&> reader) {
		if(info) { tabs(); print::out("table_switch\n"); }

		/* The index must be of type int and is popped from the operand
			stack. */
		int32 index = stack.pop_back<int32>();

		auto info_reader = reader.align_and_get_info_reader(
			[&](const uint8*& is) {
				nuint rem = (is - instructions_beginning_ptr) % 4;
				if(rem > 0) {
					is += 4 - rem;
				}
			}
		);

		auto [info, offsets_reader] = info_reader.read_and_get_offsets_reader();

		/* If index is less than low or index is greater than high, ... */
		if(index < info.low || index > info.high) {
			/* ... then a target address is calculated by adding default to
				the address of the opcode of this tableswitch instruction */
			next_instruction_ptr = instruction_ptr + info._default;
		}
		else {
			/* Otherwise, the offset at position index - low of the jump
			table is extracted */
			instr::table_switch::offset offset
				= offsets_reader.get_offset(index - info.low);

			/* The target address is calculated by adding that offset to the
			address of the opcode of this tableswitch instruction.
			Execution then continues at the target address. */
			next_instruction_ptr = instruction_ptr + offset;
		}
	}
	void operator () (instr::lookup_switch::reader<const uint8*&> reader) {
		if(info) { tabs(); print::out("lookup_switch\n"); }

		int32 key = stack.pop_back<int32>();

		auto info_reader = reader.align_and_get_info_reader(
			[&](const uint8*& is) {
				nuint rem = (is - instructions_beginning_ptr) % 4;
				if(rem > 0) {
					is += 4 - rem;
				}
			}
		);

		auto [info, offsets_reader]
			= info_reader.read_and_get_matches_and_offsets_reader();
		
		offsets_reader.read(
			[&](instr::lookup_switch::match_and_offset pair) {
				/*  The key is compared against the match values. */
				if(pair.match == key) {
					/* If it is equal to one of them, then a target address is
					calculated by adding the corresponding offset to the
					address of the opcode of this lookupswitch instruction.*/
					next_instruction_ptr = instruction_ptr + pair.offset;
					return loop_action::stop;
				}
				return loop_action::next;
			},
			/* If the key does not match any of the match values, the target
			   address is calculated by adding default to the address of the
			   opcode of this lookupswitch instruction. */
			[&] {
				next_instruction_ptr = instruction_ptr + info._default;
			}
		);
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
		if(info) { print::out(result, "\n"); }

		stack.erase_back_until(locals_begin);
		stack.emplace_back(result);

		return loop_action::stop;
	}
	loop_action operator () (instr::l_return) {
		if(info) { tabs(); print::out("l_return\n"); }
		int64 result = stack.pop_back<int64>();
		stack.erase_back_until(locals_begin);
		stack.emplace_back(result);
		return loop_action::stop;
	}
	loop_action operator () (instr::f_return) {
		if(info) { tabs(); print::out("f_return\n"); }
		float result = stack.pop_back<float>();
		stack.erase_back_until(locals_begin);
		stack.emplace_back(result);
		return loop_action::stop;
	}
	loop_action operator () (instr::d_return) {
		if(info) { tabs(); print::out("d_return\n"); }
		double result = stack.pop_back<double>();
		stack.erase_back_until(locals_begin);
		stack.emplace_back(result);
		return loop_action::stop;
	}
	loop_action operator () (instr::a_return) {
		if(info) { tabs(); print::out("a_return\n"); }
		reference result = stack.pop_back<reference>();
		stack.erase_back_until(locals_begin);
		stack.emplace_back(move(result));
		return loop_action::stop;
	}
	loop_action operator () (instr::_return) {
		if(info) { tabs(); print::out("return\n"); }
		stack.erase_back_until(locals_begin);
		return loop_action::stop;
	}
	loop_action operator () (instr::get_static x) {
		if(info) {
			tabs(); print::out("get_static ");
			class_file::constant::field_ref field_ref_const
				= c[x.field_ref_constant_index];
			class_file::constant::_class _class
				= c[field_ref_const.class_constant_index];
			class_file::constant::utf8
				class_name = c[_class.name_constant_index];
			class_file::constant::name_and_type nat
				= c[field_ref_const.name_and_type_constant_index];
			class_file::constant::utf8 name = c[nat.name_constant_index];
			print::out(class_name, ".", name, " @", stack.size(), "\n");
		}

		optional<reference> possible_throwable
			= try_get_static(c, x.field_ref_constant_index);
		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.get());
		}
		return loop_action::next;
	}
	loop_action operator () (instr::put_static x) {
		if(info) {
			tabs(); print::out("put_static ");
			class_file::constant::field_ref field_ref
				= c[x.field_ref_constant_index];
			class_file::constant::name_and_type nat
				= c[field_ref.name_and_type_constant_index];
			class_file::constant::utf8 name = c[nat.name_constant_index];
			print::out(name, "\n");
		}
		
		optional<reference> possible_throwable
			= try_put_static(m, x.field_ref_constant_index);
		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.get());
		}

		return loop_action::next;
	}
	loop_action operator () (instr::get_field x) {
		if(info) {
			tabs(); print::out("get_field ");
			auto field_ref = c[x.field_ref_constant_index];
			class_file::constant::name_and_type nat
				= c[field_ref.name_and_type_constant_index];
			class_file::constant::_class class_
				= c[field_ref.class_constant_index];
			class_file::constant::utf8 class_name
				= c[class_.name_constant_index];
			class_file::constant::utf8 field_name
				= c[nat.name_constant_index];
			print::out(class_name, ".", field_name, "\n");
		}

		optional<reference> possible_throwable
			= try_get_field(c, x.field_ref_constant_index);
		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.move());
		}
		return loop_action::next;
	}
	loop_action operator () (instr::put_field x) {
		if(info) {
			tabs(); print::out("put_field ");
			class_file::constant::field_ref field_ref
				= c[x.field_ref_constant_index];
			class_file::constant::name_and_type nat
				= c[field_ref.name_and_type_constant_index];
			class_file::constant::_class class_
				= c[field_ref.class_constant_index];
			class_file::constant::utf8 class_name
				= c[class_.name_constant_index];
			class_file::constant::utf8 name
				= c[nat.name_constant_index];
			print::out(class_name, ".", name, "\n");
		}

		optional<reference> optional_throwable
			= try_put_field(m, x.field_ref_constant_index);
		if(optional_throwable.has_value()) {
			return handle_thrown(move(optional_throwable.get()));
		}

		return loop_action::next;
	}
	loop_action operator () (instr::invoke_virtual x) {
		if(info) {
			class_file::constant::method_ref method_ref
				= c[x.method_ref_constant_index];
			class_file::constant::name_and_type nat
				= c[method_ref.name_and_type_constant_index];
			class_file::constant::utf8 desc = c[nat.descriptor_constant_index];
			class_file::constant::utf8 name = c[nat.name_constant_index];

			class_file::constant::_class c_info
				= c[method_ref.class_constant_index];
			class_file::constant::utf8 c_name
				= c[c_info.name_constant_index];

			tabs(); print::out(
				"invoke_virtual ", c_name, ".", name, desc, "\n"
			);
		}
		optional<reference> possible_throwable
			= ::try_invoke_virtual(c, x.method_ref_constant_index);

		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.move());
		}
		return loop_action::next;
	}
	loop_action operator () (
		class_file::attribute::code::instruction::invoke_special x
	) {
		if(info) {
			c.view_method_or_interface_method_constant(
				x.method_or_interface_method_ref_constant_index,
				[&](auto method_ref) {
					tabs(); print::out("invoke_special ");
					class_file::constant::name_and_type nat
						= c[method_ref.name_and_type_constant_index];
					class_file::constant::utf8 desc
						= c[nat.descriptor_constant_index];
					class_file::constant::_class c_info
						= c[method_ref.class_constant_index];
					class_file::constant::utf8 c_name
						= c[c_info.name_constant_index];
					class_file::constant::utf8 name
						= c[nat.name_constant_index];
					print::out(c_name, ".", name, desc, "\n");
				}
			);
		}

		optional<reference> possible_throwable
			= ::try_invoke_special(
				c,
				x.method_or_interface_method_ref_constant_index
			);

		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.move());
		}
		return loop_action::next;
	}
	loop_action operator () (instr::invoke_static x) {
		if(info) {
			c.view_method_or_interface_method_constant(
				x.method_or_interface_method_ref_constant_index,
				[&](auto method_ref) {
					tabs(); print::out("invoke_static ");

					class_file::constant::name_and_type nat
						= c[method_ref.name_and_type_constant_index];
					class_file::constant::utf8 desc
						= c[nat.descriptor_constant_index];
					class_file::constant::_class c_info
						= c[method_ref.class_constant_index];
					class_file::constant::utf8 c_name
						= c[c_info.name_constant_index];
					class_file::constant::utf8 name
						= c[nat.name_constant_index];

					print::out(c_name, ".", name, desc, "\n");
				}
			);

		}

		optional<reference> possible_throwable
			= ::try_invoke_static(
				c,
				x.method_or_interface_method_ref_constant_index
			);

		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.move());
		}
		return loop_action::next;
	}
	loop_action operator () (instr::invoke_interface x) {
		if(info) {
			class_file::constant::interface_method_ref method_ref
				= c[x.interface_method_ref_constant_index];
			class_file::constant::name_and_type nat
				= c[method_ref.name_and_type_constant_index];

			auto name = c[nat.name_constant_index];
			auto desc = c[nat.descriptor_constant_index];
			class_file::constant::_class c_info
				= c[method_ref.class_constant_index];
			auto c_name = c[c_info.name_constant_index];
			tabs(); print::out(
				"invoke_interface ", c_name, ".", name, desc, "\n"
			);
		}

		optional<reference> possible_throwable
			= ::try_invoke_interface(c, x.interface_method_ref_constant_index);

		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.move());
		}
		return loop_action::next;
	}
	loop_action operator () (instr::invoke_dynamic x) {
		if(info) {
			/* The run-time constant pool entry at the index must be a symbolic
			reference to a dynamically-computed call site (§5.1) */
			class_file::constant::invoke_dynamic ref
				= c[x.invoke_dynamic_constant_index];
			class_file::constant::name_and_type nat
				= c[ref.name_and_type_constant_index];
			class_file::constant::utf8 name = c[nat.name_constant_index];
			class_file::constant::utf8 desc = c[nat.descriptor_constant_index];
			tabs();
			print::out(
				"invoke_dynamic #", ref.bootstrap_method_attr_index, " ",
				name, desc, "\n"
			);
		}

		optional<reference> possible_throwable
			= ::try_invoke_dynamic(c, x.invoke_dynamic_constant_index);

		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.move());
		}
		return loop_action::next;
	}
	loop_action operator () (instr::_new x) {
		if(info) {
			tabs(); print::out("new ");
			class_file::constant::utf8 name = c[
				c[x.class_constant_index].name_constant_index
			];
			print::out(name, " @", stack.size(), "\n");
		}
		expected<::c&, reference> possible_c0
			= c.try_get_resolved_class(x.class_constant_index);

		if(possible_c0.is_unexpected()) {
			return handle_thrown(possible_c0.move_unexpected());
		}

		::c& c0 = possible_c0.get_expected();
		expected<reference, reference> possible_ref = try_create_object(c0);
		if(possible_ref.is_unexpected()) {
			return handle_thrown(possible_ref.move_unexpected());
		}
		reference ref = possible_ref.move_expected();
		stack.emplace_back(move(ref));
		return loop_action::next;
	}
	loop_action operator () (instr::new_array x) {
		optional<reference> possible_throwable
			= ::try_new_array(/* c, */ x.type);
		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.move());
		}
		return loop_action::next;
	}
	loop_action operator () (instr::a_new_array x) {
		expected<::c&, reference> possible_element_class
			= c.try_get_resolved_class(x.class_constant_index);
		
		if(possible_element_class.is_unexpected()) {
			return handle_thrown(possible_element_class.move_unexpected());
		}

		::c& element_class = possible_element_class.get_expected();

		if(info) {
			tabs(); print::out("a_new_array ");
			class_file::constant::utf8 name = element_class.name();
			print::out(name, "\n");
		}

		int32 count = stack.pop_back<int32>();
		expected<reference, reference> possible_ref
			= try_create_array_of(element_class, count);

		if(possible_ref.is_unexpected()) {
			return handle_thrown(possible_ref.move_unexpected());
		}

		reference ref = possible_ref.move_expected();

		stack.emplace_back(move(ref));
		return loop_action::next;
	}
	loop_action operator () (instr::array_length) {
		if(info) { tabs(); print::out("array_length\n"); }
		reference ref = stack.pop_back<reference>();
		if(ref.is_null()) {
			return handle_thrown(try_create_null_pointer_exception().get());
		}
		stack.emplace_back(int32{ ::array_length(ref) });
		return loop_action::next;
	}
	loop_action operator () (instr::a_throw) {
		if(info) { tabs(); print::out("a_throw\n"); }

		reference ref = stack.pop_back<reference>();
		if(ref.is_null()) {
			return handle_thrown(try_create_null_pointer_exception().get());
		}
		return handle_thrown(move(ref));
	}
	loop_action operator () (instr::check_cast x) {
		if(info) {
			class_file::constant::_class c_info = c[x.class_constant_index];
			class_file::constant::utf8 name = c[c_info.name_constant_index];
			tabs();
			print::out("check_cast ", name, "\n");
		}
		optional<reference> possible_throwable
			= ::try_check_cast(c, x.class_constant_index);
		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.move());
		}
		return loop_action::next;
	}
	loop_action operator () (instr::instance_of x) {
		optional<reference> possible_throwable
			= ::try_check_instance_of(c, x.class_constant_index);
		if(possible_throwable.has_value()) {
			return handle_thrown(possible_throwable.move());
		}
		return loop_action::next;
	}
	loop_action operator () (instr::monitor_enter) {
		reference ref = stack.pop_back<reference>();
		if(ref.is_null()) {
			return handle_thrown(try_create_null_pointer_exception().get());
		}
		ref->lock();
		return loop_action::next;
	}
	loop_action operator () (instr::monitor_exit) {
		reference ref = stack.pop_back<reference>();
		if(ref.is_null()) {
			return handle_thrown(try_create_null_pointer_exception().get());
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