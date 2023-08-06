#pragma once

#include "decl/class.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/info.hpp"
#include "decl/lib/java/lang/invoke/call_site.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include <class_file/constant.hpp>

#include <print/print.hpp>

[[nodiscard]] inline optional<reference> try_invoke_dynamic(
	c& c, class_file::constant::invoke_dynamic_index ref_index
) {
	/* The symbolic reference is resolved (§5.4.3.6) for this specific
	   invokedynamic instruction to obtain a reference to an instance of
	   java.lang.invoke.CallSite. The instance of java.lang.invoke.CallSite is
	   considered "bound" to this specific invokedynamic instruction. */
	// In our case, CallSite is bound to constant table entry, TODO
	expected<reference, reference> possible_call_site
		= c.try_get_resolved_call_site(ref_index);
	
	if(possible_call_site.is_unexpected()) {
		return possible_call_site.move_unexpected();
	}

	reference cs_ref = possible_call_site.move_expected();
	j::call_site& cs = (j::call_site&) cs_ref.object();

	reference mh_ref = cs.get_target();
	j::method_handle& mh = (j::method_handle&) mh_ref.object();

	return mh.try_invoke_exact();
}