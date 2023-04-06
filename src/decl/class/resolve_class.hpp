#pragma once

#include "class.hpp"
#include "classes.hpp"

#include <range.hpp>

/* To resolve an unresolved symbolic reference from D to a class or interface C
   denoted by N, the following steps are performed: */
template<basic_range Name>
inline expected<_class&, reference> try_resolve_class(_class& d, Name&& name) {
	/* 1. The defining loader of D is used to load and thereby create a class
	      or interface denoted by N. This class or interface is C. The details
	      of the process are given in §5.3. */
	reference defining_loader = d.defining_loader();
	expected<_class&, reference> possible_c
		= classes.try_load_class(name, defining_loader.object_ptr());

	if(possible_c.is_unexpected()) {
		return { possible_c.get_unexpected() };
	}
	_class& c = possible_c.get_expected();
	
	/*    Any exception that can be thrown as a result of failure to load and
	      thereby create C can thus be thrown as a result of failure of class
	      and interface resolution.*/ // TODO

	/* 2. If C is an array class and its element type is a reference type, then
	      a symbolic reference to the class or interface representing the
	      element type is resolved by invoking the algorithm in §5.4.3.1
	      recursively. */
	if(c.is_array()) {
		nuint dimensionality = 0;
		for(char c : name) {
			if(c == '[') ++dimensionality;
			else break;
		}
		char element_type = name[dimensionality];
		bool element_is_reference = element_type == 'L';

		if(element_is_reference) {
			auto element_name = iterator_and_sentinel {
				name.iterator() + dimensionality + 1, // skip 'L'
				name.sentinel() - 1 // ';'
			}.as_range();
			expected<_class&, reference> possible_c
				= try_resolve_class(c, element_name);
			if(possible_c.is_unexpected()) {
				return unexpected{ move(possible_c.get_unexpected()) };
			}
		}
	}
	/* 3. Finally, access control is applied for the access from D to C
	      (§5.4.4). */ // TODO
	return c;
}