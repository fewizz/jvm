#include "decl/lib/jvm/mh/static_getter.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_static_getter() {
	mh_static_getter_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"jvm/mh/StaticGetter" }
	);

	mh_static_getter_constructor
		= mh_static_getter_class->declared_instance_methods().find(
			c_string{ u8"<init>" },
			c_string{ u8"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;S)V" }
		);

	mh_static_getter_class->declared_instance_methods().find(
		c_string{ u8"invokeExactPtr" }, c_string{ u8"()V" }
	).native_function(
		(void*)+[](
			j::method_handle& ths0
		) -> optional<reference> {
			jvm::class_member& ths = (jvm::class_member&) ths0;

			static_field& resolved_field
				= ths.member<declared_static_field_index>();
			
			optional<reference> optional_throwable
				= resolved_field.c().try_initialise_if_need();
			if(optional_throwable.has_value()) {
				return move(optional_throwable.get());
			}

			get_static_resolved(resolved_field);

			return {};
		}
	);
}