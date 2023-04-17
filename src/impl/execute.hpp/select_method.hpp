#include "decl/class.hpp"
#include "decl/method.hpp"

#include <loop_action.hpp>

// 5.4.6. Method Selection
/* During execution of an invokeinterface or invokevirtual instruction,
   a method is selected with respect to (i) the run-time type of the object on
   the stack, and (ii) a method that was previously resolved by the
   instruction. The rules to select a method with respect to a class or
   interface C and a method mR are as follows: */
inline optional<method&> try_select_method(
	_class& c, method& mr
) {
	/* 1. If mR is marked ACC_PRIVATE, then it is the selected method. */
	if(mr.access_flags()._private) {
		return mr;
	}
	/* 2. Otherwise, the selected method is determined by the following
	      lookup procedure: " */
	/*    • If C contains a declaration of an instance method m that can
	        override mR (§5.4.5), then m is the selected method" */

	for(method& m : c.declared_instance_methods()) {
		if(m.can_override(mr)) {
			return m;
		}
	}

	/*   • Otherwise, if C has a superclass, a search for a declaration of an
	       instance method that can override mR is performed, starting with the
	       direct superclass of C and continuing with the direct superclass of
	       that class, and so forth, until a method is found or no further
	       superclasses exist. If a method is found, it is the selected
	       method. */
	{
		_class* s = &c;
		while(s->has_super()) {
			s = &s->super();

			for(method& m : s->declared_instance_methods()) {
				if(m.can_override(mr)) {
					return m;
				}
			}
		}
	}

	/*   • Otherwise, the maximally-specific superinterface methods of C are
	       determined (§5.4.3.3). If exactly one matches mR's name and
	       descriptor and is not abstract, then it is the selected method. */
	optional<method&> m;
	c.for_each_maximally_specific_super_interface_instance_method(
		mr.name(), mr.descriptor(),
		[&](method& m0) {
			if(!m0.access_flags().abstract) {
				if(m.has_value()) {
					m = {};
					return loop_action::stop;
				}
				m = m0;
			}
			return loop_action::next;
		}
	);
	
	return m;
}