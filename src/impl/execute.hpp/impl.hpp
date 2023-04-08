#include "decl/execute.hpp"

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
#include <class_file/attribute/code/read_instruction.hpp>

#include <print/print.hpp>

static optional<reference> try_execute(method& m) {
	_class& c = m._class();
	if(info) {
		tabs();
		print::out("executing: ", c.name(), ".", m.name(), m.descriptor());
		++tab;
	}

	on_scope_exit bring_tab_back {
		[] { if(info) { --tab; } }
	};

	on_scope_exit print_finish { [&] {
		if(info) {
			tabs();
			print::out(
				"finishing executing: ",
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
		if(info) {
			print::out("\n");
		}

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
					thrown._class().name(),
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

	if(info) {
		print::out(" ",
			"max_stack: ", m.code().max_stack,
			" locals begin: ", locals_begin,
			" stack begin: ", stack_begin, "\n"
		);
	}

	{
		nuint max_possible_stack_end = stack_begin + m.code().max_stack * 2;
		if(max_possible_stack_end > stack.capacity()) {
			stack.pop_back_until(locals_begin);
			expected<reference, reference> possible_soe
				= try_create_stack_overflow_error();

			return move(
				possible_soe.is_unexpected() ?
				possible_soe.get_unexpected() :
				possible_soe.get_expected()
			);
		}
	}

	{
		nuint locals_pushed = m.parameters_stack_size();
		while(locals_pushed < m.code().max_locals) {
			stack.emplace_back(int32{ -1 });
			++locals_pushed;
		}
	}

	const uint8* instruction_ptr = m.code().iterator();
	const uint8* instruction_end_ptr = m.code().sentinel();

	reference thrown;

	while(instruction_ptr < instruction_end_ptr) {
		const uint8* next_instruction_ptr = instruction_ptr;

		loop_action action = class_file::attribute::code::instruction::read(
			next_instruction_ptr,
			// beginning, required for table_swith and others
			m.code().iterator(),
			[&]<typename Type>(Type instruction) -> loop_action {
				on_scope_exit update_instruction_ptr{[&] {
					instruction_ptr = next_instruction_ptr;
				}};

				ctx.instruction_ptr = instruction_ptr;

				execute_instruction instr_exe {
					.m = m,
					.c = c,
					.instruction_ptr = instruction_ptr,
					// TODO
					.next_instruction_ptr = (const uint8*&)next_instruction_ptr,
					.locals_begin = locals_begin,
					.stack_begin = stack_begin,
					.thrown = thrown
				};

				if constexpr(
					same_as<decltype(instr_exe(instruction)), loop_action>
				) {
					return instr_exe(instruction);
				}
				else {
					instr_exe(instruction);

					return loop_action::next;
				}
			}
		);

		if(action == loop_action::stop) break;
	}

	if(!thrown.is_null()) {
		return thrown;
	}

	return {};
}