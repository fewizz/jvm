#pragma once

#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/conversion.hpp"

namespace mh {

template<
	range_of<c&> T0Params,
	range_of<c&> T1Params
>
[[nodiscard]] inline bool check_non_varargs(
	T0Params&& t0_params, c& t0_ret,
	T1Params&& t1_params, c& t1_ret
) {
	if(range_size(t1_params) != range_size(t0_params)) {
		return false;
	}

	for(nuint i = 0; i < range_size(t1_params); ++i) {
		c& t0_param = t0_params[i];
		c& t1_param = t1_params[i];

		bool convertible = is_conversion_allowed<
			method_handle_invoke_context
		>(t0_param, t1_param);

		if(!convertible) {
			return false;
		}
	}

	// conversion is always allowed if one of tX_ret is void
	if(!t0_ret.is(void_class.get()) && !t1_ret.is(void_class.get())) {
		return is_conversion_allowed<method_handle_invoke_context>(
			t1_ret, t0_ret
		);
	}

	return true;
}

template<
	range_of<c&> T0Params,
	range_of<c&> T1Params
>
[[nodiscard]] inline bool check_varargs(
	T0Params&& t0_params, c& t0_ret,
	T1Params&& t1_params, c& t1_ret
) {
	nuint t0_params_count = range_size(t0_params);
	nuint t1_params_count = range_size(t1_params);

	nuint last_t1_param_index = t1_params_count - 1;
	c& last_t1_param_type = t1_params[last_t1_param_index];
	c& component_type = last_t1_param_type.get_component_class();

	for(
		nuint param_index = last_t1_param_index;
		param_index < t0_params_count;
		++param_index
	) {
		c& t0_param = t0_params[param_index];

		bool convertible = is_conversion_allowed<
			assignment_context
		>(t0_param, component_type);

		if(!convertible) {
			return false;
		}
	}

	return check_non_varargs(
		range{ t0_params }.shrink_view(last_t1_param_index), t0_ret,
		range{ t1_params }.shrink_view(last_t1_param_index), t1_ret
	);
}

template<
	range_of<c&> T0Params,
	range_of<c&> T1Params
>
[[nodiscard]] inline bool check(
	T0Params&& t0_params, c& t0_ret,
	T1Params&& t1_params, c& t1_ret,
	bool t1_is_varargs
) {
	return
		!t1_is_varargs ?
		check_non_varargs(
			t0_params, t0_ret,
			t1_params, t1_ret
		)
		:
		check_varargs(
			t0_params, t0_ret,
			t1_params, t1_ret
		);
}

[[nodiscard]] inline bool check(
	j::method_type& t0_mt,
	j::method_type& t1_mt,
	bool t1_is_varargs
) {
	auto t0_params = t0_mt.parameter_types_view();
	auto t1_params = t1_mt.parameter_types_view();

	c& t0_ret = t0_mt.return_type();
	c& t1_ret = t1_mt.return_type();

	return check(
		t0_params, t0_ret,
		t1_params, t1_ret,
		t1_is_varargs
	);
}

} // mh