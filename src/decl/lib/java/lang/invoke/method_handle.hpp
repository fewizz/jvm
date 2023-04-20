#pragma once

#include "decl/class.hpp"
#include "decl/descriptor.hpp"
#include "decl/primitives.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/lib/java/lang/object.hpp"

#include "decl/lib/java/lang/boolean.hpp"
#include "decl/lib/java/lang/byte.hpp"
#include "decl/lib/java/lang/short.hpp"
#include "decl/lib/java/lang/character.hpp"
#include "decl/lib/java/lang/integer.hpp"
#include "decl/lib/java/lang/long.hpp"
#include "decl/lib/java/lang/float.hpp"
#include "decl/lib/java/lang/double.hpp"

#include <optional.hpp>
#include <overloaded.hpp>

static optional<_class&> method_handle_class;

inline instance_method_index method_handle_invoke_exact_ptr_index;

[[nodiscard]] inline optional<reference>
method_handle_invoke_exact(reference mh_ref, nuint args_beginning);

template<typename T0, typename T1>
inline bool method_handle_is_parameter_type_convertible(
	_class& new_, _class& original
) {
	// If T0 and T1 are references
	if constexpr(same_as<T0, reference> && same_as<T1, reference>) {
		return can_cast(new_, original);
	}
	// If T0 and T1 are primitives
	if constexpr(
		(
			same_as<T0, int8> &&
			same_as_any<T1, int16, int32, int64, float, double>
		) ||
		(
			same_as_any<T0, int16, uint16> &&
			same_as_any<T1, int32, int64, float, double>
		) ||
		( same_as<T0, int32> && same_as_any<T1, int64, float, double> ) ||
		( same_as<T0, int64> && same_as_any<T1, float, double> ) ||
		( same_as<T0, float> && same_as<T1, double> )
	) {
		return true;
	}
	if constexpr(same_as<T0, T1>) {
		return true;
	}
	// If T0 is a primitive and T1 a reference
	if constexpr(!same_as<T0, reference> && same_as<T1, reference>) {
		if(
			original.is(object_class.get()) ||
			(same_as<T0, int8> && original.is(java_lang_byte_class.get())) ||
			(same_as<T0, int16> && original.is(java_lang_short_class.get())) ||
			(
				same_as<T0, uint16> &&
				original.is(java_lang_character_class.get())
			) ||
			(
				same_as<T0, int32> &&
				original.is(java_lang_integer_class.get())
			) ||
			(same_as<T0, int64> && original.is(java_lang_long_class.get())) ||
			(same_as<T0, float> && original.is(java_lang_float_class.get())) ||
			(same_as<T0, double> && original.is(java_lang_double_class.get()))
		) {
			return true;
		}
	}
	/* If T0 is a reference and T1 a primitive, an unboxing conversion will be
	   applied at runtime, possibly followed by a Java method invocation
	   conversion (JLS 5.3) on the primitive value. */
	if constexpr(same_as<T0, reference> && !same_as<T1, reference>) {
		
	}
	return false;
}

inline bool method_handle_is_parameter_type_convertible(
	_class& new_,
	_class& original
) {
	return new_.view_raw_type([&]<typename A>() -> bool {
		return original.view_raw_type([&]<typename B>() -> bool {
			return method_handle_is_parameter_type_convertible<A, B>(
				new_, original
			);
		});
	});
}

inline optional<reference>
method_handle_try_convert(
	_class& original,
	_class& new_
) {
	original.view_raw_type([&]<typename A>() {
		new_.view_raw_type([&]<typename B>() {
			if constexpr(
				same_as<A, B>
			) {
				return;
			} else if constexpr(
				(    // byte to short, int, long, float, dobule
					same_as<A, int8> &&
					same_as_any<B, int16, int32, int64, float, double>
				) || // short and char to int, long, float, dobule
				(
					same_as_any<A, int16, uint16> &&
					same_as_any<B, int32, int64, float, double>
				)
			) {
				stack.emplace_back<B>(stack.pop_back<int32>());
			} else if constexpr(
				     // int to long, float, dobule
				(
					same_as<A, int32> && same_as_any<B, int64, float, double>
				) || // long to float, dobule
				(
					same_as<A, int64> && same_as_any<B, float, double>
				) || // float to dobule
				(
					same_as<A, float> && same_as<B, double>
				)
			) {
				stack.emplace_back<B>(stack.pop_back<A>());
			}

			posix::abort(); // impossible
		});
	});

	return {};
}