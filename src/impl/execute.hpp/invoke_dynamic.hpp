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

	if(info) {
		cc::name_and_type nat {
			c.name_and_type_constant(ref.name_and_type_index)
		};
		cc::utf8 name = c.utf8_constant(nat.name_index);
		cc::utf8 desc = c.utf8_constant(nat.descriptor_index);
		tabs(); fputs("invoke_dynamic #", stderr);
		fprintf(stderr, "%hu", ref.bootstrap_method_attr_index);
		fputc(' ', stderr);
		fwrite(name.elements_ptr(), 1, name.size(), stderr);
		fwrite(desc.elements_ptr(), 1, desc.size(), stderr);
		fputc('\n', stderr);
	}

	reference call_site = c.get_call_site(ref_index);

	fputs("unimplemented", stderr); abort();
}