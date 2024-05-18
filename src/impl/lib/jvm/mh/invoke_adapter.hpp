#include "decl/lib/jvm/mh/invoke_adapter.hpp"

#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"

static void init_jvm_mh_invoke_adapter() {
	jvm::invoke_adapter::c = classes.load_class_by_bootstrap_class_loader(
		u8"jvm/mh/InvokeAdapter"sv
	);

	jvm::invoke_adapter::constructor =
		jvm::invoke_adapter::c->declared_instance_methods()
		.find(
			u8"<init>"sv,
			u8"("
				"Ljava/lang/invoke/MethodType;"
				"Z"
				"Ljava/lang/invoke/MethodHandle;"
			")V"sv
		);

	jvm::invoke_adapter::original_field_position
		= jvm::invoke_adapter::c->instance_field_position(
			u8"original_"sv,
			u8"Ljava/lang/invoke/MethodHandle;"sv
		);
	
	jvm::invoke_adapter::c->declared_instance_methods()
	.find(u8"check"sv, u8"()Z"sv)
	.native_function(
		(void*)+[](native_environment*, jvm::invoke_adapter* new_mh) -> bool {
			j::method_type& new_mt = new_mh->method_type();

			j::method_handle& ori_mh = new_mh->original();
			j::method_type& ori_mt = ori_mh.method_type();

			return mh::check(new_mt, ori_mt, ori_mh.is_varargs());
		}
	);

	jvm::invoke_adapter::c->declared_instance_methods()
	.find(u8"invokeExactPtr"sv, u8"()V"sv)
	.native_function(
		(void*)+[](
			j::method_handle& ths
		) -> optional<reference> {
			jvm::invoke_adapter& t0_mh = (jvm::invoke_adapter&) ths;

			j::method_handle& t1_mh = t0_mh.original();

			return mh::try_invoke_unchecked(
				t0_mh.method_type(), t1_mh
			);
		}
	);

}