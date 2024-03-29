#include "decl/lib/jvm/mh/virtual.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/native/environment.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_virtual() {
	mh_virtual_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"jvm/mh/Virtual" }
	);

	mh_virtual_constructor = mh_virtual_class->declared_instance_methods().find(
		c_string{ u8"<init>" },
		c_string {
			u8"("
				"Ljava/lang/invoke/MethodType;"
				"Z"
				"Ljava/lang/Class;"
				"S"
			")V"
		}
	);

	mh_virtual_class->declared_instance_methods().find(
		c_string{ u8"invokeExactPtr" }, c_string{ u8"()V" }
	).native_function(
		(void*)+[](j::method_handle& ths0)
		-> optional<reference> {
			jvm::class_member& ths = (jvm::class_member&) ths0;

			method& resolved_method
				= ths.member<declared_instance_method_index>();

			return try_invoke_virtual_resolved_non_polymorphic(resolved_method);
		}
	);

}