#include "decl/lib/jvm/mh/static.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/native/environment.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_static() {
	mh_static_class = classes.load_class_by_bootstrap_class_loader(
		u8"jvm/mh/Static"s
	);

	mh_static_constructor = mh_static_class->declared_instance_methods().find(
		u8"<init>"s,
		u8"("
			"Ljava/lang/invoke/MethodType;"
			"Z"
			"Ljava/lang/Class;"
			"S"
		")V"s
	);

	mh_static_class->declared_instance_methods().find(
		u8"invokeExactPtr"s, u8"()V"s
	).native_function(
		(void*)+[](
			j::method_handle& ths0
		) -> optional<reference> {
			jvm::class_member& ths = (jvm::class_member&) ths0;

			static_method& resolved_method
				= ths.member<declared_static_method_index>();

			optional<reference> init_error
				= resolved_method.c().try_initialise_if_need();

			if(init_error.has_value()) {
				return move(init_error.get());
			}

			return try_invoke_static_resolved(resolved_method);
		}
	);

}