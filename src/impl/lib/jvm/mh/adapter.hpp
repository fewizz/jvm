#include "decl/classes.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"

static layout::position jvm_mh_adapter_original_field_position;

static void init_jvm_mh_adapter() {
	_class& c = classes.load_class_by_bootstrap_class_loader(
		c_string{"jvm/mh/Adapter"}
	);

	jvm_mh_adapter_original_field_position = c.instance_field_position(
		c_string{"original_"}, c_string{"Ljava/lang/invoke/MethodHandle;"}
	);

	c.declared_methods()
	.find(c_string{"()V"}, c_string{"invokeExactPtr"})
	.native_function(
		(void*)+[](
			reference new_mh,
			[[maybe_unused]] nuint args_beginning
		) -> optional<reference> {
			object& new_mt = new_mh->get<reference>(
				method_handle_method_type_field_position
			);
			object& new_params0 = new_mt.get<reference>(
				method_type_parameter_types_instance_field_position
			);
			object& new_ret = new_mt.get<reference>(
				method_type_return_type_instance_field_position
			);

			object& ori_mh = new_mh->get<reference>(
				jvm_mh_adapter_original_field_position
			);
			object& ori_mt = ori_mh.get<reference>(
				method_handle_method_type_field_position
			);
			object& ori_params0 = ori_mt.get<reference>(
				method_type_parameter_types_instance_field_position
			);
			object& ori_ret = ori_mt.get<reference>(
				method_type_return_type_instance_field_position
			);

			auto new_params = array_as_span<reference>(new_params0);
			auto ori_params = array_as_span<reference>(ori_params0);

			for(nuint i = 0; i < new_params.size(); ++i) {
				_class& new_p = class_from_class_instance(new_params[i]);
				_class& ori_p = class_from_class_instance(ori_params[i]);

				optional<reference> possible_throwable
					= method_handle_try_convert_on_stack(new_p, ori_p);
				
				if(possible_throwable.has_value()) {
					return move(possible_throwable.get());
				}
			}

			optional<reference> possible_throwable
				= method_handle_invoke_exact(ori_mh, args_beginning);
			
			if(possible_throwable.has_value()) {
				return move(possible_throwable.get());
			}

			possible_throwable
				= method_handle_try_convert_on_stack(
					class_from_class_instance(ori_ret),
					class_from_class_instance(new_ret)
				);

			return {};
		}
	);
}