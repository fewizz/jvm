#pragma once

#include "info.hpp"
#include "stack_entry.hpp"
#include "declaration.hpp"
#include "../class/declaration.hpp"
#include "../method/declaration.hpp"
#include "class/file/code/instruction.hpp"
#include "class/file/descriptor/reader.hpp"

inline void invoke_special(
	_class& c, class_file::code::instruction::invoke_special x,
	stack_entry* stack, nuint& stack_size
) {
	namespace cc = class_file::constant;

	cc::method_ref method_ref = c.method_ref_constant(x.index);
	cc::name_and_type nat {
		c.name_and_type_constant(method_ref.name_and_type_index)
	};
	
	cc::utf8 method_desc = c.utf8_constant(nat.descriptor_index);

	if(info) {
		tabs(); fputs("invoke_special ", stderr);
		cc::_class _c = c.class_constant(method_ref.class_index);
		cc::utf8 class_name = c.utf8_constant(_c.name_index);
		fwrite(class_name.data(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		cc::utf8 method_name = c.utf8_constant(nat.name_index);
		fwrite(method_name.data(), 1, method_name.size(), stderr);
		fwrite(method_desc.data(), 1, method_desc.size(), stderr);
		fputc('\n', stderr);
	}

	class_file::descriptor::method_reader params_reader{ method_desc.begin() };
	uint16 args_count = 0;
	params_reader([&](auto){ ++args_count; return true; });
	++args_count; // this
	stack_size -= args_count;
	method_with_class wic = c.get_resolved_method(x.index);
	stack_entry result = execute(
		wic, span{ stack + stack_size, args_count }
	);
	if(!result.is<jvoid>()) {
		stack[stack_size++] = result;
	}
}