#pragma once

#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/conversion.hpp"

namespace mh {

[[nodiscard]] inline optional<reference>
try_convert_return_value_on_stack(
	c& t0_ret, c& t1_ret
) {
	// T0 and T1 are reversed here (vs javadoc)

	bool t0_is_void = t0_ret.is(void_class.get());
	bool t1_is_void = t1_ret.is(void_class.get());

	if(t0_is_void && t1_is_void) {
		return {};
	}

	/* If the return type T1 is marked as void, any returned value is
	   discarded */
	if(!t1_is_void && t0_is_void) {
		t1_ret.view_non_void_raw_type([]<typename T1>() {
			stack.erase_back<T1>();
		});
		return {};
	}
	if(t1_is_void && !t0_is_void) {
		/* If the return type T0 is void and T1 a reference, a null value is
		   introduced. */
		if(t0_ret.is_reference()) {
			stack.emplace_back(reference{});
		}
		/* If the return type T0 is void and T1 a primitive, a zero value is
		   introduced. */
		else {
			t0_ret.view_non_void_raw_type([]<typename T0>() {
				stack.emplace_back(T0{});
			});
		}
		return {};
	}

	return t1_ret.view_non_void_raw_type([&]<typename T1>()
	-> optional<reference> {
		T1 t1 = stack.pop_back<T1>();

		return try_convert_on_stack<
			method_handle_invoke_context
		>(move(t1), t1_ret, t0_ret);
	});
}

template<range_of<c&> T0Params, range_of<c&> T1Params>
[[nodiscard]] inline optional<reference>
try_convert_arguments_on_stack(
	T0Params&& t0_params, T1Params&& t1_params, nuint index
) {
	--index;

	c& t0_param = t0_params[index];
	c& t1_param = t1_params[index];

	return t0_param.view_non_void_raw_type([&]<typename T0>()
	-> optional<reference> {
		T0 t0 = stack.pop_back<T0>();

		if(index > 0) {
			optional<reference> possible_throwable
				= try_convert_arguments_on_stack(
					t0_params,
					t1_params,
					index
				);
			if(possible_throwable.has_value()) {
				return possible_throwable.move();
			}
		}

		return try_convert_on_stack<
			method_handle_invoke_context
		>(move(t0), t0_param, t1_param);
	});
}

template<range_of<c&> T0Params, range_of<c&> T1Params>
[[nodiscard]] inline optional<reference>
try_convert_arguments_on_stack(
	T0Params&& t0_params, T1Params&& t1_params
) {
	auto size = range_size(t0_params);
	if(size == 0) return {};

	return try_convert_arguments_on_stack(
		forward<T0Params>(t0_params),
		forward<T1Params>(t1_params),
		size
	);
}

}