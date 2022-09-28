#pragma once

#include "decl/class.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/info.hpp"
#include "decl/print.hpp"

#include <class_file/constant.hpp>

template<basic_range StackType>
inline void invoke_dynamic(
	class_file::constant::invoke_dynamic_index ref_index,
	_class& c, [[maybe_unused]] StackType& stack
) {
	namespace cc = class_file::constant;

	cc::invoke_dynamic ref = c.invoke_dynamic_constant(ref_index);

	if(info) {
		cc::name_and_type nat {
			c.name_and_type_constant(ref.name_and_type_index)
		};
		cc::utf8 name = c.utf8_constant(nat.name_index);
		cc::utf8 desc = c.utf8_constant(nat.descriptor_index);
		tabs(); print("invoke_dynamic #");
		print(ref.bootstrap_method_attr_index);
		print(" ");
		print(name);
		print(desc);
		print("\n");
	}

	reference call_site = c.get_call_site(ref_index);

	posix::std_err.write_from(c_string{ "unimplemented\n" });
	abort();
}