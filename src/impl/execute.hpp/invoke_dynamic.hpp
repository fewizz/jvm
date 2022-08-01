#pragma once

#include "class.hpp"
#include "execution/stack.hpp"
#include "execution/info.hpp"

#include <class_file/constant.hpp>

inline void invoke_dynamic(
	class_file::constant::invoke_dynamic_index ref_index,
	_class& c, stack& stack
) {
	namespace cc = class_file::constant;

	cc::invoke_dynamic ref = c.invoke_dynamic_constant(ref_index);
	cc::name_and_type nat {
		c.name_and_type_constant(ref.name_and_type_index)
	};
	
	cc::utf8 method_name = c.utf8_constant(nat.name_index);
	cc::utf8 method_desc = c.utf8_constant(nat.descriptor_index);

	if(info) {
		tabs(); fputs("invoke_dynamic #", stderr);
		fprintf(stderr, "%hu", ref.bootstrap_method_attr_index);

		fputc(' ', stderr);

		fwrite(method_name.data(), 1, method_name.size(), stderr);
		fwrite(method_desc.data(), 1, method_desc.size(), stderr);

		fputc('\n', stderr);
	}

	fputs("unimplemented", stderr); abort();
}