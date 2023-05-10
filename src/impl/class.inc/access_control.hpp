#include "decl/class/access_control.hpp"

#include "decl/class.hpp"
#include "decl/class/member.hpp"
#include "decl/lib/java/lang/illegal_access_error.hpp"

/* Access control is applied during resolution (§5.4.3) to ensure that a
   reference to a class, interface, field, or method is permitted. Access
   control succeeds if a specified class, interface, field, or method is
   accessible to the referring class or interface. */

[[nodiscard]] inline optional<reference>
access_control(c& d, c& c) {
	/* A class or interface C is accessible to a class or interface D if and
	   only if one of the following is true: */
	
	/* * C is public, and a member of the same run-time module as D (§5.3.6). */
	bool public_and_same_module =
		c.is_public(); // TODO check module
	
	/* * C is public, and a member of a different run-time module than D, and
	     C's runtime module is read by D's run-time module, and C's run-time
	     module exports C's run-time package to D's run-time module. */
	bool c_exports_d = false;// TODO

	/* * C is not public, and C and D are members of the same run-time
	     package. */
	bool same_package =
		!c.is_public() &&
		c.package().has_equal_size_and_elements(d.package());

	bool accessible = public_and_same_module + c_exports_d + same_package == 1;

	/* If C is not accessible to D, then access control throws an
	   IllegalAccessError. */
	if(!accessible) {
		return try_create_illegal_access_error().get();
	}

	/* Otherwise, access control succeeds. */
	return {};
}

[[nodiscard]] inline optional<reference>
access_control(c& d, class_member& r) {
	c& c = r.c();

	/* A field or method R is accessible to a class or interface D if and only
	   if any of the following is true: */

	/* * R is public. */
	bool r_is_public = r.is_public();

	/* * R is protected and is declared in a class C, and D is either a subclass
	     of C or C itself. */
	bool r_is_protected =
		r.is_protected() && (d.is_sub_of(c) || d.is(c));
	
	/* * R is either protected or has default access (that is, neither public
	     nor protected nor private), and is declared by a class in the same
	     run-time package as D. */
	bool r_is_protected_or_default_same_package =
		(r.is_protected() || r.has_default_access()) &&
		c.package().has_equal_size_and_elements(d.package());
	
	/* * R is private and is declared by a class or interface C that belongs to
	     the same nest as D, according to the nestmate test below. */
	bool r_is_private =
		r.is_private() &&
		r.c().is(c); // TODO, temporaly

	bool accessible =
		r_is_public ||
		r_is_protected ||
		r_is_protected_or_default_same_package ||
		r_is_private;

	/* If R is not accessible to D, then access control throws an
	   IllegalAccessError. */
	if(!accessible) {
		return try_create_illegal_access_error().get();
	}

	/* Otherwise, access control succeeds. */
	return {};
}