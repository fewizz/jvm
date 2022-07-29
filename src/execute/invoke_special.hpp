#pragma once

#include "./method_ref_index.hpp"
#include "execute.hpp"
#include "execution/info.hpp"
#include "execution/stack_entry.hpp"

#include "class.hpp"
#include "method.hpp"

inline optional<reference> invoke_special(
	method_ref_index ref_index, _class& c,
	stack_entry* stack, nuint& stack_size
) {
	namespace cc = class_file::constant;

	cc::method_ref method_ref = c.method_ref_constant(ref_index);
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

	method_with_class wic = c.get_resolved_method(ref_index);

	auto args_count = wic.method().arguments_count();
	++args_count; // this
	stack_size -= args_count;

	expected<stack_entry, reference> result = execute(
		wic,
		arguments_container{ stack + stack_size, args_count }
	);

	if(result.is_unexpected()) {
		return result.get_unexpected();
	}

	if(!result.get_expected().is<jvoid>()) {
		stack[stack_size++] = result;
	}

	return {};
}