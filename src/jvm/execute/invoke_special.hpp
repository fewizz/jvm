#pragma once

#include "stack_entry.hpp"
#include "../class.hpp"
#include "../method.hpp"

inline void invoke_special(
	_class& c, class_file::code::instruction::invoke_special x,
	stack_entry* stack, nuint& stack_size
) {
	method& m0 = c.get_resolved_method(x.index);
	auto desc = m0.descriptor();

	if(info) {
		tabs(); fputs("invoke_special ", stderr);
		auto name = m0._class().name();
		fwrite(name.data(), 1, name.size(), stderr);
		fputc('.', stderr);
		fwrite(m0.name().data(), 1, m0.name().size(), stderr);
		fputc('\n', stderr);
	}

	class_file::descriptor::method_reader params_reader{ desc.begin() };
	uint16 args_count = 0;
	params_reader([&](auto){ ++args_count; return true; });
	++args_count; // this
	stack_size -= args_count;
	stack_entry result = execute(
		m0, span{ stack + stack_size, args_count }
	);
	if(!result.is<jvoid>()) {
		stack[stack_size++] = result;
	}
}