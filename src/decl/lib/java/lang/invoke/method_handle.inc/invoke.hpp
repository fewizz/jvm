#pragma once

#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "./convert.hpp"

namespace mh {

template<range_of<c&> T0Params>
[[nodiscard]] optional<reference> try_invoke_unchecked(
	T0Params&& t0_params,
	c& t0_ret,
	j::method_handle& t1_mh
) {
	j::method_type& t1_mt = t1_mh.method_type();

	c& t1_ret = t1_mt.return_type();

	auto t1_params = t1_mt.parameter_types_view();

	optional<reference> possible_throwable
		= try_convert_arguments_on_stack(
			t0_params, t1_params
		);

	if(possible_throwable.has_value()) {
		return possible_throwable.move();
	}

	possible_throwable = t1_mh.try_invoke_exact();

	if(possible_throwable.has_value()) {
		return possible_throwable.move();
	}

	possible_throwable
		= try_convert_return_value_and_save_on_stack(
			t1_ret, t0_ret
		);

	if(possible_throwable.has_value()) {
		return possible_throwable.move();
	}

	return {};
}

[[nodiscard]] inline optional<reference> try_invoke_unchecked(
	j::method_type& t0_mt,
	j::method_handle& t1_mh
) {
	auto t0_params = t0_mt.parameter_types_view();
	c& t0_ret = t0_mt.return_type();
	return try_invoke_unchecked(t0_params, t0_ret, t1_mh);
}

}