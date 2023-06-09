#pragma once

#include "./identity.hpp"
#include "./widening_primitive.hpp"
#include "./widening_reference.hpp"
#include "./boxing.hpp"
#include "./unboxing.hpp"
#include "./cast_in_runtime.hpp"

#include <types.hpp>

struct assignment_context {

	using conversions = types<
		identity_conversion,
		widening_primitive_conversion,
		widening_reference_conversion,
		widening_reference_conversion::
			followed_by_unboxig_conversion,
		widening_reference_conversion::
			followed_by_unboxig_conversion::
				followed_by_widening_primitive_conversion,
		boxing_conversion,
		boxing_conversion::
			followed_by_widening_reference_conversion,
		unboxing_conversion,
		unboxing_conversion::
			followed_by_widening_primitive_conversion
	>;

};

struct strict_invocation_context {

	using conversions = types<
		identity_conversion,
		widening_primitive_conversion,
		widening_reference_conversion
	>;

};

struct loose_invocation_context {

	using conversions = types<
		identity_conversion,
		widening_primitive_conversion,
		widening_reference_conversion,
		widening_reference_conversion::
			followed_by_unboxig_conversion,
		widening_reference_conversion::
			followed_by_unboxig_conversion::
				followed_by_widening_primitive_conversion,
		boxing_conversion,
		boxing_conversion::
			followed_by_widening_reference_conversion,
		unboxing_conversion,
		unboxing_conversion::
			followed_by_widening_primitive_conversion
	>;

};

struct method_handle_invoke_context {

	using conversions = types<
		/* If T0 and T1 are references, then a cast to T1 is applied. (The types
		   do not need to be related in any particular way. This is because a
		   dynamic value of null can convert to any reference type.) */
		cast_in_runtime_conversion,
		/* If T0 and T1 are primitives, then a Java method invocation conversion
		   (JLS 5.3) is applied, if one exists. (Specifically, T0 must convert
		   to T1 by a widening primitive conversion.) */
		widening_primitive_conversion,
		/* If T0 is a primitive and T1 a reference, a Java casting conversion
		   (JLS 5.5) is applied if one exists. (Specifically, the value is boxed
		   from T0 to its wrapper class, which is then widened as needed to
		   T1.) */
		boxing_conversion::followed_by_widening_reference_conversion,
		/* If T0 is a reference and T1 a primitive, an unboxing conversion will
		   be applied at runtime, possibly followed by a Java method invocation
		   conversion (JLS 5.3) on the primitive value. (These are the primitive
		   widening conversions.) T0 must be a wrapper class or a supertype of
		   one. (In the case where T0 is Object, these are the conversions
		   allowed by java.lang.reflect.Method.invoke.) The unboxing conversion
		   must have a possibility of success, which means that if T0 is not
		   itself a wrapper class, there must exist at least one wrapper class
		   TW which is a subtype of T0 and whose unboxed primitive value can be
		   widened to T1. */
		unboxing_conversion::followed_by_widening_primitive_conversion
	>;

};