#pragma once

#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "./convert.hpp"

namespace mh {

[[nodiscard]] inline optional<reference> try_invoke_checked(
	object& ori_mh,
	object_of<jl::i::method_type>& new_mt,
	object_of<jl::i::method_type>& ori_mt,
	nuint args_beginning
) {
	c& new_ret = new_mt.return_type();
	c& ori_ret = ori_mt.return_type();

	auto new_params = new_mt.parameter_types_view();
	auto ori_params = ori_mt.parameter_types_view();

	optional<reference> possible_throwable
		= try_convert_arguments_on_stack(
			new_params, ori_params
		);
	if(possible_throwable.has_value()) {
		return possible_throwable.move();
	}

	possible_throwable = mh::try_invoke_exact(ori_mh, args_beginning);
	
	if(possible_throwable.has_value()) {
		return possible_throwable.move();
	}

	possible_throwable
		= try_convert_return_value_and_save_on_stack(
			new_ret, ori_ret
		);

	return {};
}

}