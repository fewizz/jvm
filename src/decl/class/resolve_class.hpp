#pragma once

#include "class.hpp"
#include "classes.hpp"
#include <range.hpp>

template<basic_range Name>
inline _class& resolve_class([[maybe_unused]]_class& d, Name&& name) {
	/* To resolve an unresolved symbolic reference from D to a class or
	   interface C denoted by N, the following steps are performed: */
	/* 1. The defining loader of D is used to load and thereby create a class
	      or interface denoted by N. This class or interface is C. The details
	      of the process are given in §5.3. */
	// TODO
	_class& c = classes.find_or_load(name);
	/* 2. If C is an array class and its element type is a reference type, then
	      a symbolic reference to the class or interface representing the
	      element type is resolved by invoking the algorithm in §5.4.3.1
	      recursively. */
	if(c.is_array()) {
		c.get_component_class(); // TODO
	}
	/* 3. Finally, access control is applied for the access from D to C
	      (§5.4.4). */ // TODO
	return c;
}