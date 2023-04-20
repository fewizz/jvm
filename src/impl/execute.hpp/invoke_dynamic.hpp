#pragma once

#include "decl/class.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/info.hpp"

#include <class_file/constant.hpp>

#include <print/print.hpp>

[[nodiscard]] inline optional<reference> try_invoke_dynamic(
	_class& c, class_file::constant::invoke_dynamic_index ref_index
) {
	namespace cc = class_file::constant;

	if(info) {
		/* The run-time constant pool entry at the index must be a symbolic
		   reference to a dynamically-computed call site (§5.1) */
		cc::invoke_dynamic ref = c.invoke_dynamic_constant(ref_index);
		cc::name_and_type nat {
			c.name_and_type_constant(ref.name_and_type_index)
		};
		cc::utf8 name = c.utf8_constant(nat.name_index);
		cc::utf8 desc = c.utf8_constant(nat.descriptor_index);
		tabs();
		print::out(
			"invoke_dynamic #", ref.bootstrap_method_attr_index, " ",
			name, desc, "\n"
		);
	}

	/* The symbolic reference is resolved (§5.4.3.6) for this specific
	   invokedynamic instruction to obtain a reference to an instance of
	   java.lang.invoke.CallSite. The instance of java.lang.invoke.CallSite is
	   considered "bound" to this specific invokedynamic instruction. */
	// In our case, CallSite is bound to constant table entry
	expected<reference, reference> possible_call_site
		= c.try_get_resolved_call_site(ref_index);
	
	if(possible_call_site.is_unexpected()) {
		return move(possible_call_site.get_unexpected());
	}

	print::err("unimplemented\n");
	posix::abort();
}