#pragma once

#include "./decl.hpp"
#include "./info.hpp"
#include "./stack_entry.hpp"

#include "class/decl.hpp"

#include <class/file/constant.hpp>
#include <class/file/attribute/code/instruction.hpp>

inline optional<reference> invoke_interface(
	_class& c, class_file::attribute::code::instruction::invoke_interface x,
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
		cc::_class class_info {
			c.class_constant(method_ref_info.class_index)
		};
		auto class_name = c.utf8_constant(class_info.name_index);
		tabs(); fputs("invoke_interface ", stderr);
		fwrite(class_name.data(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		fwrite(name.data(), 1, name.size(), stderr);
		fwrite(desc.data(), 1, desc.size(), stderr);
		fputc('\n', stderr);
	}

	uint16 args_count = x.count;

	optional<method&> m0{};
	optional<_class&> c0 = stack[stack_size - args_count]
		.get<reference>().object()._class();

	while(true) {
		if(m0 = c0->try_find_method(name, desc); m0.has_value()) {
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
			name, desc,
			[&](method_with_class m) {
				if(index++ == 0) {
					m0 = m.method();
					c0 = m._class();
					return;
				}
				fputs(
					"more than one maximally-specific interface method", stderr
				);
				abort();
			}
		);
	}

	if(!m0.has_value()) {
		fputs("couldn't find method", stderr); abort();
	}

	stack_size -= args_count;
	expected<stack_entry, reference> result = execute(
		method_with_class{ m0.value(), c0.value() },
		args_container{ stack + stack_size, args_count }
	);

	if(result.is_unexpected()) {
		return result.get_unexpected();
	}

	if(!result.get_expected().is<jvoid>()) {
		stack[stack_size++] = result;
	}

	return {};
}