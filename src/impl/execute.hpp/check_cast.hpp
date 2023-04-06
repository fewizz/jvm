#pragma once

#include "decl/class.hpp"
#include "decl/lib/java/lang/object.hpp"
#include "decl/lib/java/lang/class_cast_exception.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"

#include <loop_action.hpp>

inline optional<reference>
try_check_cast(_class& c, class_file::constant::class_index index) {
	expected<_class&, reference> possible_t = c.try_get_resolved_class(index);
	
	if(possible_t.is_unexpected()) {
		return possible_t.get_unexpected();
	}

	_class& t = possible_t.get_expected();

	if(info) {
		tabs();
		print::out("check_cast ", t.name(), "\n");
	}

	reference& ref = stack.back<reference>();
	if(ref.is_null()) {
		return {};
	}
	
	struct cast {

	bool operator () (_class& s, _class& t) {
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
		/* If T is a class type, then T must be Object. */
		if(!t.is_interface() && !t.is_array()) {
			return t.is(object_class.get());
		}
		/* "If T is an interface type, then T must be one of the interfaces
		    implemented by arrays (JLS §4.10.3)." */
		if(t.is_interface()) {
			return s.is_implementing(t);
		}
		/* "If T is an array type TC[], that is, an array of components of
		    type TC, then one of the following must be true:" */
		_class& sc = s.get_component_class();
		_class& tc = t.get_component_class();
		/* "TC and SC are the same primitive type." */
		if(sc.is_primitive() && tc.is_primitive() && sc.is(tc)) {
			return true;
		}
		/* "TC and SC are reference types, and type SC can be cast to TC
		    by recursive application of these rules." */
		if(sc.is_reference() && tc.is_reference() && cast{}(sc, tc)) {
			return true;
		}
		return false;
	}};

	_class& s = ref._class();

	/* "If objectref can be cast to the resolved class, array, or interface
	    type, the operand stack is unchanged; otherwise, the checkcast
	    instruction throws a ClassCastException." */
	if(!cast{}(s, t)) {
		expected<reference, reference> possible_exception
			= try_create_class_cast_exception();
		return move(
			possible_exception.is_unexpected() ?
			possible_exception.get_unexpected() :
			possible_exception.get_expected()
		);
	}

	return {};
}