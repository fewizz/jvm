#pragma once

#include "info.hpp"
#include "stack_entry.hpp"
#include "../class.hpp"
#include "../method.hpp"
#include "class/file/code/instruction.hpp"

inline void invoke_static(
	_class& c, class_file::code::instruction::invoke_static x,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		namespace cc = class_file::constant;

		fputs("invoke_static ", stderr);

		cc::method_ref ref = c.method_ref_constant(x.index);
		auto class_name {
			c.utf8_constant(c.class_constant(ref.class_index).name_index)
		};
		fwrite(class_name.data(), 1, class_name.size(), stderr);

		fputc('.', stderr);

		cc::name_and_type nat {
			c.name_and_type_constant(ref.name_and_type_index)
		};

		auto name = c.utf8_constant(nat.name_index);
		fwrite(name.data(), 1, name.size(), stderr);

		auto desc = c.utf8_constant(nat.descriptor_index);
		fwrite(desc.data(), 1, desc.size(), stderr);

		fputc('\n', stderr);
	}

	method& next_method = c.get_method(x.index);
	auto desc = next_method.descriptor();

	class_file::descriptor::method_reader params_reader{ desc.begin() };
	uint16 args_count = 0;
	params_reader([&](auto){ ++args_count; return true; });

	stack_size -= args_count;
	stack_entry result = execute(
		next_method,
		span{ stack + stack_size, args_count }
	);
	if(!result.is<jvoid>()) {
		stack[stack_size++] = result;
	}
}