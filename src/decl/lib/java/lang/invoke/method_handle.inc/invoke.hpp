#pragma once

#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "./convert.hpp"

namespace mh {

template<range_of<c&> T0Params>
[[nodiscard]] optional<reference> try_invoke_unchecked_non_varargs(
	T0Params&& t0_params,
	c& t0_ret,
	j::method_handle& t1_mh
) {
	j::method_type& t1_mt = t1_mh.method_type();

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

	c& t1_ret = t1_mt.return_type();

	possible_throwable
		= try_convert_return_value_and_save_on_stack(
			t1_ret, t0_ret
		);

	if(possible_throwable.has_value()) {
		return possible_throwable.move();
	}

	return {};
}

template<range_of<c&> T0Params>
[[nodiscard]] optional<reference> try_invoke_unchecked(
	T0Params&& t0_params,
	c& t0_ret,
	j::method_handle& t1_mh
) {
	if(!t1_mh.is_varargs()) {
		return try_invoke_unchecked_non_varargs(
			forward<T0Params>(t0_params), t0_ret, t1_mh
		);
	}

	j::method_type& t1_mt = t1_mh.method_type();
	auto t1_params = t1_mt.parameter_types_view();
	nuint t0_params_count = range_size(t0_params);
	nuint t1_params_count = range_size(t1_params);
	nuint varargs_index = t1_params_count - 1;
	c& t1_last_parameter = t1_params[varargs_index];

	if(
		t0_params_count < varargs_index ||
		!t1_last_parameter.is_array()
	) {
		posix::abort();
	}

	{ // collect args to array
		c& array_component = t1_last_parameter.get_component_class();

		nuint varargs_length = t0_params_count - varargs_index;
		expected<reference, reference> possible_varargs_array
			= try_create_array_of(array_component, varargs_length);
		if(possible_varargs_array.is_unexpected()) {
			return possible_varargs_array.move_unexpected();
		}

		reference varargs_array_ref = possible_varargs_array.move_expected();

		array_component.view_raw_type_non_void([&]<typename Type>() {
			span<Type> varargs_array = array_as_span<Type>(varargs_array_ref);

			nuint varargs_left = varargs_length;
			while(varargs_left > 0) {
				--varargs_left;

				Type t = stack.pop_back<Type>();
				varargs_array[varargs_left] = move(t);
			}
		});

		stack.emplace_back(move(varargs_array_ref));
	}

	return view_on_stack<c*>{ t1_params_count }(
		[&](span<c*> new_t0_params
	) -> decltype(auto) {
		for(nuint p = 0; p < varargs_index; ++p) {
			new_t0_params[p] = &t0_params[p];
		}
		new_t0_params[varargs_index] = &t1_params[varargs_index];

		return try_invoke_unchecked_non_varargs(
			new_t0_params.dereference_view(), t0_ret, t1_mh
		);
	});
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