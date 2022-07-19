#pragma once

#include "./decl.hpp"
#include "./info.hpp"
#include "./stack_entry.hpp"

#include "../class/decl.hpp"
#include "../method/decl.hpp"

#include <class/file/attribute/code/instruction.hpp>
#include <class/file/descriptor/reader.hpp>

inline optional<reference> invoke_static(
	_class& c, class_file::attribute::code::instruction::invoke_static x,
	stack_entry* stack, nuint& stack_size
) {
	namespace cc = class_file::constant;

	cc::method_ref method_ref = c.method_ref_constant(x.index);
	cc::name_and_type nat {
		c.name_and_type_constant(method_ref.name_and_type_index)
	};
	cc::utf8 method_desc = c.utf8_constant(nat.descriptor_index);

	if(info) {
		tabs(); fputs("invoke_static ", stderr);
		cc::_class _c = c.class_constant(method_ref.class_index);
		cc::utf8 class_name = c.utf8_constant(_c.name_index);
		fwrite(class_name.data(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		auto method_name = c.utf8_constant(nat.name_index);
		fwrite(method_name.data(), 1, method_name.size(), stderr);
		fwrite(method_desc.data(), 1, method_desc.size(), stderr);
		fputc('\n', stderr);
	}

	class_file::descriptor::method_reader params_reader{ method_desc.begin() };
	uint16 args_count = 0;
	params_reader([&](auto){ ++args_count; return true; });
	stack_size -= args_count;
	method_with_class wic = c.get_static_method(x.index);

	expected<stack_entry, reference> result = execute(
		wic,
		span{ stack + stack_size, args_count }
	);

	if(result.is_unexpected()) {
		return result.get_unexpected();
	}

	if(!result.get_expected().is<jvoid>()) {
		stack[stack_size++] = result;
	}

	return {};
}