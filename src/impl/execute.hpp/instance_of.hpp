#pragma once

#include "decl/class.hpp"
#include "decl/lib/java/lang/object.hpp"
#include "decl/lib/java/lang/class_cast_exception.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"

#include <loop_action.hpp>

#include <print/print.hpp>

inline optional<reference> try_check_instance_of(
	c& c, class_file::constant::class_index index
) {
	expected<::c&, reference> possible_t = c.try_get_resolved_class(index);
	if(possible_t.is_unexpected()) {
		return { possible_t.get_unexpected() };
	}

	::c& t = possible_t.get_expected();
	
	if(info) {
		tabs();
		print::out("instance_of ", t.name());
	}

	struct check {

	bool operator () (::c& s, ::c& t) {
		/* "If S is a class type, then:" */
		if(!s.is_array()) {
			/* "If T is a class type, then S must be the same class as T, or S
			    must be a subclass of T;" */
			if(!t.is_interface() && !t.is_array()) {
				return s.is(t) || s.is_sub_of(t);
			}
			/* "If T is an interface type, then S must implement interface T."*/
			if(t.is_interface()) {
				return s.is_implementing(t);
			}
			return false;
		}
		/* "If S is an array type SC[], that is, an array of components of type
		    SC, then:" */
		/* "If T is a class type, then T must be Object." */
		if(!t.is_interface() && !t.is_array()) {
			return t.is(object_class.get());
		}
		/* "If T is an interface type, then T must be one of the interfaces
		    implemented by arrays (JLS §4.10.3)." */
		if(t.is_interface()) {
			return s.is_implementing(t);
		}
		/* "If T is an array type TC[], that is, an array of components of type
		    TC, then one of the following must be true:" */
		::c& sc = s.get_component_class();
		::c& tc = t.get_component_class();
		/* "TC and SC are the same primitive type." */
		if(sc.is_primitive() && tc.is_primitive() && sc.is(tc)) {
			return true;
		}
		/* "TC and SC are reference types, and type SC can be cast to TC by
		    these run-time rules. "*/
		if(sc.is_reference() && tc.is_reference() && check{}(sc, tc)) {
			return true;
		}
		return false;
	}};

	int32 result;
	reference& ref = stack.back<reference>();
	/* "If objectref is null, the instanceof instruction pushes an int result of
	    0 as an int onto the operand stack." */
	if(ref.is_null()) {
		result = 0;
	}
	/* "Otherwise, the named class, array, or interface type is resolved
	    (§5.4.3.1). If objectref is an instance of the resolved class or array
	    type, or implements the resolved interface, the instanceof instruction
	    pushes an int result of 1 as an int onto the operand stack; otherwise,
	    it pushes an int result of 0. */
	else {
		::c& s = ref.c();
		result = check{}(s, t);
	}

	if(info) {
		print::out(" ref type: ");
		if(ref.is_null()) {
			print::out("null");
		}
		else {
			print::out(ref.c().name());
		}
		print::out(" ", result != 0, "\n");
	}

	stack.emplace_back(result);
	return {};
}