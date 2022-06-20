#pragma once

#include "declaration.hpp"
#include "info.hpp"
#include "stack_entry.hpp"
#include "../class/declaration.hpp"
#include "../object/declaration.hpp"
#include "class/file/constant.hpp"
#include "class/file/code/instruction.hpp"
#include "class/file/descriptor/reader.hpp"

inline void invoke_virtual(
	_class& c, class_file::code::instruction::invoke_virtual x,
	stack_entry* stack, nuint& stack_size
) {
	namespace cf = class_file;
	namespace cc = cf::constant;

	cc::method_ref method_ref { c.method_ref_constant(x.index) };
	cc::name_and_type nat {
		c.name_and_type_constant(method_ref.name_and_type_index)
	};
	cc::utf8 method_name = c.utf8_constant(nat.name_index);
	cc::utf8 method_desc = c.utf8_constant(nat.descriptor_index);

	if(info) {
		cc::_class _c { c.class_constant(method_ref.class_index) };
		cc::utf8 class_name = c.utf8_constant(_c.name_index);
		tabs(); fputs("invoke_virtual ", stderr);
		fwrite(class_name.data(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		fwrite(method_name.data(), 1, method_name.size(), stderr);
		fwrite(method_desc.data(), 1, method_desc.size(), stderr);
		fputc('\n', stderr);
	}

	cf::descriptor::method_reader params_reader{ method_desc.begin() };
	uint16 args_count = 0;
	params_reader([&](auto){ ++args_count; return true; });

	reference& ref = stack[stack_size - args_count - 1].get<reference>();
	optional<_class&> c0 = ref.object()._class();
	optional<method&> m0{};

	while(true) {
		if(m0 = c0->try_find_method(method_name, method_desc); m0.has_value()) {
			break;
		}
		if(!c0->has_super_class()) {
			break;
		}
		c0 = c0->super_class();
	}

	if(!m0.has_value()) {
		nuint index = 0;
		c.for_each_maximally_specific_superinterface_method(
			method_name, method_desc,
			[&](method_with_class mwc) {
				if(index++ == 0) {
					m0 = mwc.method;
					c0 = mwc._class;
					return;
				}
				fputs("more than one maximally-specific", stderr);
				abort();
			}
		);
	}

	if(!m0.has_value()) {
		fputs("couldn't find method", stderr); abort();
	}

	++args_count; // this
	stack_size -= args_count;
	stack_entry result = execute(
		method_with_class{ m0.value(), c0.value() },
		span{ stack + stack_size, args_count }
	);
	if(!result.is<jvoid>()) {
		stack[stack_size++] = result;
	}
}