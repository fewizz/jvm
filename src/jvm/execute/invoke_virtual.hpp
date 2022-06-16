#pragma once

#include "definition.hpp"
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

	cc::method_ref method_ref_info { c.method_ref_constant(x.index) };
	cc::name_and_type name_and_type_info {
		c.name_and_type_constant(method_ref_info.name_and_type_index)
	};
	auto name = c.utf8_constant(name_and_type_info.name_index);
	auto desc = c.utf8_constant(name_and_type_info.descriptor_index);

	if(info) {
		cc::_class class_info { c.class_constant(method_ref_info.class_index) };
		auto class_name = c.utf8_constant(class_info.name_index);
		tabs(); fputs("invoke_virtual ", stderr);
		fwrite(class_name.data(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		fwrite(name.data(), 1, name.size(), stderr);
		fwrite(desc.data(), 1, desc.size(), stderr);
		fputc('\n', stderr);
	}

	cf::descriptor::method_reader params_reader{ desc.begin() };
	uint16 args_count = 0;
	params_reader([&](auto){ ++args_count; return true; });

	optional<method&> m0{};
	optional<_class&> c0 = stack[stack_size - args_count - 1]
		.get<reference>().object()._class();

	while(true) {
		if(m0 = c0->try_find_method(name, desc); m0.has_value()) {
			break;
		}
		if(c0->super_class_index() == 0) {
			break;
		}
		c0 = c0->get_class(c0->super_class_index());
	}

	if(!m0.has_value()) {
		nuint index = 0;
		c.for_each_maximally_specific_superinterface_method(
			name, desc,
			[&](method& m) {
				if(index++ == 0) {
					m0 = m;
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
		m0.value(), span{ stack + stack_size, args_count }
	);
	if(!result.is<jvoid>()) {
		stack[stack_size++] = result;
	}
}