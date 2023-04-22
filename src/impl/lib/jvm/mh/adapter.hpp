#include "decl/lib/jvm/mh/adapter.hpp"

#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"

static layout::position jvm_mh_adapter_original_field_position;

static void init_jvm_mh_adapter() {
	mh_adapter_class = classes.load_class_by_bootstrap_class_loader(
		c_string{"jvm/mh/Adapter"}
	);

	mh_adapter_constructor =
		mh_adapter_class->declared_instance_methods()
		.find(
			c_string{"<init>"},
			c_string {
				"("
					"Ljava/lang/invoke/MethodType;"
					"Ljava/lang/invoke/MethodHandle;"
				")V"
			}
		);

	jvm_mh_adapter_original_field_position
		= mh_adapter_class->instance_field_position(
			c_string{"original_"}, c_string{"Ljava/lang/invoke/MethodHandle;"}
		);
	
	mh_adapter_class->declared_instance_methods()
	.find(c_string{"check"}, c_string{"()Z"})
	.native_function(
		(void*)+[](native_environment*, object* new_mh) -> bool {
			object& new_mt = new_mh->get<reference>(
				method_handle_method_type_field_position
			);
			_class& new_ret = method_type_return_type(new_mt);

			object& ori_mh = new_mh->get<reference>(
				jvm_mh_adapter_original_field_position
			);
			object& ori_mt = ori_mh.get<reference>(
				method_handle_method_type_field_position
			);
			_class& ori_ret = method_type_return_type(ori_mt);

			auto new_params = method_type_parameter_types_view(new_mt);
			auto ori_params = method_type_parameter_types_view(ori_mt);

			if(new_params.size() != ori_params.size()) {
				return false;
			}

			for(nuint i = 0; i < new_params.size(); ++i) {
				_class& new_p = new_params[i];
				_class& ori_p = ori_params[i];

				if(!method_handle_is_type_convertible(new_p, ori_p)) {
					return false;
				}
			}

			return method_handle_is_type_convertible(ori_ret, new_ret);
		}
	);

	mh_adapter_class->declared_instance_methods()
	.find(c_string{"invokeExactPtr"}, c_string{"()V"})
	.native_function(
		(void*)+[](
			reference new_mh,
			[[maybe_unused]] nuint args_beginning
		) -> optional<reference> {
			object& new_mt = new_mh->get<reference>(
				method_handle_method_type_field_position
			);
			_class& new_ret = method_type_return_type(new_mt);

			object& ori_mh = new_mh->get<reference>(
				jvm_mh_adapter_original_field_position
			);
			object& ori_mt = ori_mh.get<reference>(
				method_handle_method_type_field_position
			);
			_class& ori_ret = method_type_return_type(ori_mt);

			auto new_params = method_type_parameter_types_view(new_mt);
			auto ori_params = method_type_parameter_types_view(ori_mt);

			optional<reference> possible_throwable
				= method_handle_try_convert_arguments_on_stack(
					new_params, ori_params
				);
			if(possible_throwable.has_value()) {
				return move(possible_throwable.get());
			}

			possible_throwable
				= method_handle_invoke_exact(ori_mh, args_beginning);
			
			if(possible_throwable.has_value()) {
				return move(possible_throwable.get());
			}

			possible_throwable
				= method_handle_try_convert_return_value_and_save_on_stack(
					ori_ret, new_ret
				);

			return {};
		}
	);
}