#pragma once

#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "./convert.hpp"

namespace mh {

[[nodiscard]] inline optional<reference> try_invoke_checked(
object& ori_mh, object& new_mt, object& ori_mt, nuint args_beginning
) {
	c& new_ret = method_type_return_type(new_mt);
	c& ori_ret = method_type_return_type(ori_mt);

	auto new_params = method_type_parameter_types_view(new_mt);
	auto ori_params = method_type_parameter_types_view(ori_mt);

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