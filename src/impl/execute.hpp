#include "decl/execute.hpp"

#include "./execute.inc/instruction.hpp"

#include "./execute.inc/select_method.hpp"
#include "./execute.inc/select_method_for_invoke_special.hpp"

#include "./execute.inc/invoke_dynamic.hpp"
#include "./execute.inc/invoke_interface.hpp"
#include "./execute.inc/invoke_special.hpp"
#include "./execute.inc/invoke_static.hpp"
#include "./execute.inc/invoke_virtual.hpp"

#include "./execute.inc/get_field.hpp"
#include "./execute.inc/put_field.hpp"

#include "./execute.inc/get_static.hpp"
#include "./execute.inc/put_static.hpp"

#include "decl/execution/info.hpp"
#include "decl/execution/context.hpp"
#include "decl/execution/latest_context.hpp"
#include "decl/native/call.hpp"
#include "decl/class.hpp"
#include "decl/lib/java/lang/stack_overflow_error.hpp"

#include <class_file/reader.hpp>
#include <class_file/descriptor/method.hpp>
#include <class_file/attribute/code/read_instruction.hpp>

#include <print/print.hpp>

static optional<reference> try_execute(method& m) {
	c& c = m.c();
	if(info) {
		tabs();
		print::out("executing: ", c.name(), ".", m.name(), m.descriptor(), "\n");
		++tab;
	}

	on_scope_exit print_finish { [&] {
		if(info) {
			--tab;
			tabs();
			print::out(
				"finishing execution of: ",
				c.name(), ".", m.name(), m.descriptor(),
				"\n"
			);
		}
	}};

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
			posix::abort();
		}
		native_function_ptr ptr = m.native_function();

		optional<reference> possible_throwable
			= try_native_interface_call(ptr, m);

		if(possible_throwable.has_value()) {
			reference thrown = possible_throwable.get();
			if(info) {
				tabs();
				print::out(
					"uncatched exception ",
					thrown.c().name(),
					"\n"
				);
			}
			return move(thrown);
		}

		return {};
	}

	if(m.code().iterator() == nullptr) {
		print::err("no code\n");
		posix::abort();
	}

	nuint locals_begin = stack.size() - m.parameters_stack_size();
	nuint locals_end = locals_begin + m.code().max_locals;
	nuint stack_begin = locals_end;
	nuint stack_end = stack_begin + m.code().max_stack;

	if(info) {
		tabs();
		print::out(
			"locals: ", locals_begin, "-", stack_begin, ", "
			"stack: ", stack_begin, "-", stack_end, "\n"
		);
	}

	{
		nuint max_possible_stack_end = stack_end * 2;
		if(max_possible_stack_end > stack.capacity()) {
			stack.erase_back_until(locals_begin);
			return try_create_stack_overflow_error().get();
		}
	}

	{
		nuint locals_pushed = m.parameters_stack_size();
		while(locals_pushed < m.code().max_locals) {
			stack.emplace_back(int32{ -1 });
			++locals_pushed;
		}
	}

	execute_instruction exe {
		.m = m,
		.c = c,
		.instructions_beginning_ptr = m.code().iterator(),
		.instruction_ptr = m.code().iterator(),
		.next_instruction_ptr = m.code().iterator(),
		.locals_begin = locals_begin,
		.stack_begin = stack_begin
	};

	while(true) {
		loop_action action = class_file::attribute::code::instruction::read(
			exe.next_instruction_ptr,
			[&]<typename Type>(Type instruction) -> loop_action {
				on_scope_exit update_instruction_ptr{[&] {
					exe.instruction_ptr = exe.next_instruction_ptr;
				}};

				ctx.instruction_ptr = exe.instruction_ptr;

				if constexpr(
					same_as<decltype(exe(instruction)), loop_action>
				) {
					return exe(instruction);
				}
				else {
					exe(instruction);
					return loop_action::next;
				}
			}
		);

		if(action == loop_action::stop) {
			break;
		}
	}

	if(!exe.thrown.is_null()) {
		return move(exe.thrown);
	}

	return {};
}