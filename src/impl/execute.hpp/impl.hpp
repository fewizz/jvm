#include "decl/execute.hpp"

#include "./thrown.hpp"
#include "./instruction.hpp"

#include "./select_method.hpp"
#include "./select_method_for_invoke_special.hpp"

#include "execution/info.hpp"
#include "execution/context.hpp"
#include "execution/latest_context.hpp"
#include "native/call.hpp"
#include "class.hpp"
#include "lib/java/lang/stack_overflow_error.hpp"

#include <class_file/reader.hpp>
#include <class_file/descriptor/method_reader.hpp>
#include <class_file/attribute/code/reader.hpp>

static void execute(method& m) {
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

	class_file::attribute::code::reader<
		uint8*,
		class_file::attribute::code::reader_stage::code
	> code_reader{ m.code().iterator() - sizeof(uint32) }; // TODO, messy

	const uint8* instrution_ptr = m.code().iterator();

	code_reader.read_and_get_exception_table_reader(
	[&]<typename Type>(Type x0, uint8*& next_instruction_ptr)
	-> loop_action {
		on_scope_exit update_instruction_ptr{[&] {
			instrution_ptr = next_instruction_ptr;
		}};

		execute_instruction instr_exe {
			.m = m,
			.c = c,
			.instruction_ptr = instrution_ptr,
			// TODO
			.next_instruction_ptr = (const uint8*&) next_instruction_ptr,
			.locals_begin = locals_begin,
			.stack_begin = stack_begin
		};

		if constexpr(same_as<decltype(instr_exe(x0)), loop_action>) {
			return instr_exe(x0);
		}
		else {
			instr_exe(x0);
			return loop_action::next;
		}
	}, m.code().size());
}