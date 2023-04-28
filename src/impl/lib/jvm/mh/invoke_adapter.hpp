#include "decl/lib/jvm/mh/invoke_adapter.hpp"

#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"

static layout::position jvm_mh_invoke_adapter_original_field_position;

static void init_jvm_mh_invoke_adapter() {
	mh_invoke_adapter_class = classes.load_class_by_bootstrap_class_loader(
		c_string{"jvm/mh/InvokeAdapter"}
	);

	mh_invoke_adapter_constructor =
		mh_invoke_adapter_class->declared_instance_methods()
		.find(
			c_string{"<init>"},
			c_string {
				"("
					"Ljava/lang/invoke/MethodType;"
					"Ljava/lang/invoke/MethodHandle;"
				")V"
			}
		);

	jvm_mh_invoke_adapter_original_field_position
		= mh_invoke_adapter_class->instance_field_position(
			c_string{"original_"}, c_string{"Ljava/lang/invoke/MethodHandle;"}
		);
	
	mh_invoke_adapter_class->declared_instance_methods()
	.find(c_string{"check"}, c_string{"()Z"})
	.native_function(
		(void*)+[](native_environment*, object* new_mh) -> bool {
			object& new_mt = new_mh->get<reference>(
				method_handle_method_type_field_position
			);

			object& ori_mh = new_mh->get<reference>(
				jvm_mh_invoke_adapter_original_field_position
			);
			object& ori_mt = ori_mh.get<reference>(
				method_handle_method_type_field_position
			);

			return method_handle_are_types_convertible(new_mt, ori_mt);
		}
	);

	mh_invoke_adapter_class->declared_instance_methods()
	.find(c_string{"invokeExactPtr"}, c_string{"()V"})
	.native_function(
		(void*)+[](
			reference new_mh,
			[[maybe_unused]] nuint args_beginning
		) -> optional<reference> {
			object& new_mt = new_mh->get<reference>(
				method_handle_method_type_field_position
			);

			object& ori_mh = new_mh->get<reference>(
				jvm_mh_invoke_adapter_original_field_position
			);
			object& ori_mt = ori_mh.get<reference>(
				method_handle_method_type_field_position
			);

			return method_handle_try_invoke_checked(
				ori_mh, new_mt, ori_mt, args_beginning
			);
		}
	);
}