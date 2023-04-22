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
inline layout::position method_handle_method_type_field_position;

[[nodiscard]] inline optional<reference>
method_handle_invoke_exact(reference mh_ref, nuint args_beginning);

template<typename T0, typename T1>
[[nodiscard]] inline bool method_handle_is_type_convertible(
	_class& t0, _class& t1
) {
	if constexpr(same_as<T0, void_t> || same_as<T1, void_t>) {
		return true;
	}
	/* If T0 and T1 are references */
	if constexpr(same_as<T0, reference> && same_as<T1, reference>) {
		return can_cast(t0, t1);
	}
	/* If T0 and T1 are primitives */
	if constexpr(widening_conversion_allowed<T0, T1>) {
		return true;
	}
	if constexpr(same_as<T0, T1>) {
		return true;
	}
	/* If T0 is a primitive and T1 a reference */
	if constexpr(
		not_same_as<void_t, T0> &&
		not_same_as<T0, reference> &&
		same_as<T1, reference>
	) {
		return
		t1.is(object_class.get()) ||
		t1.is(wrapper_class_by_primitive_type<T0>());
	}
	/* If T0 is a reference and T1 a primitive, an unboxing conversion will be
	   applied at runtime, possibly followed by a Java method invocation
	   conversion (JLS 5.3) on the primitive value. */
	if constexpr(
		not_same_as<T1, void_t> &&
		same_as<T0, reference> &&
		not_same_as<T1, reference>
	) {
		return
		/* In the case where T0 is Object, these are the conversions allowed
			by java.lang.reflect.Method.invoke. */
		t0.is(object_class.get()) ||
		t0.is(wrapper_class_by_primitive_type<T1>());
	}

	return false;
}

[[nodiscard]] inline bool method_handle_is_type_convertible(
	_class& t0,
	_class& t1
) {
	return t0.view_raw_type([&]<typename T0>() -> bool {
		return t1.view_raw_type([&]<typename T1>() -> bool {
			return method_handle_is_type_convertible<T0, T1>(
				t0, t1
			);
		});
	});
}

template<typename T0, typename T1>
[[nodiscard]] inline optional<reference>
method_handle_try_convert_on_stack(
	[[maybe_unused]] _class& t0,
	[[maybe_unused]] _class& t1
) {
	/* If T0 and T1 are references */
	if constexpr(same_as<reference, T0> && same_as<reference, T1>) {
		return {};
	}
	/* If T0 and T1 are primitives */
	if constexpr(widening_conversion_allowed<T0, T1>) {
		if constexpr(
			not_same_as<T0, void_t> && not_same_as<T0, void_t>
		) {
			stack.emplace_back((T1)stack.pop_back<T0>());
			return {};
		}
	}
	if constexpr( // both primitives of the same type, skip
		not_same_as<reference, T0> && not_same_as<reference, T1> &&
		same_as<T0, T1>
	) {
		return {};
	}
	/* If T0 is a primitive and T1 a reference, a Java casting
	   conversion (JLS 5.5) is applied if one exists. */
	if constexpr(
		not_same_as<reference, T0> && same_as<reference, T1>
	) {
		// TODO clang is dumb? or me? (probably second)
		if constexpr(
			not_same_as<T0, void_t> &&
			not_same_as<reference, T0>
		) {
			optional<reference> possible_exception
				= try_box_on_stack<T0>();
			if(possible_exception.has_value()) {
				return move(possible_exception.get());
			}

			return {};
		}
	}
	/* If T0 is a reference and T1 a primitive, an unboxing conversion
	   will be applied at runtime, possibly followed by a Java method
	   invocation conversion (JLS 5.3) on the primitive value. */
	if constexpr(
		not_same_as<T1, void_t> &&
		same_as<T0, reference> && not_same_as<T1, reference>
	) {

		unbox_on_stack<T1>();

		return {};
	}
	/* If the return type T1 is marked as void, any returned value is
	   discarded */
	if constexpr(same_as<T1, void_t>) {
		if constexpr(not_same_as<T0, void_t>) {
			stack.pop_back<T0>();
			return {};
		}
	}
	/* If the return type T0 is void and T1 a reference, a null value is
	   introduced. */
	if constexpr(same_as<T0, void_t> && same_as<T1, reference>) {
		stack.emplace_back(reference{});
		return {};
	}
	/* If the return type T0 is void and T1 a primitive, a zero value is
	   introduced. */
	if constexpr(same_as<T0, void_t> && not_same_as<T1, reference>) {
		if constexpr(not_same_as<T1, void_t>) {
			stack.emplace_back(T1{});
			return {};
		}
	}

	posix::abort(); // impossible
}

[[nodiscard]] inline optional<reference>
method_handle_try_convert_on_stack(
	_class& t0,
	_class& t1
) {
	t0.view_raw_type([&]<typename T0>() -> optional<reference> {
		return t1.view_raw_type([&]<typename T1>() -> optional<reference> {
			return method_handle_try_convert_on_stack<T0, T1>(t0, t1);
		});
	});

	return {};
}