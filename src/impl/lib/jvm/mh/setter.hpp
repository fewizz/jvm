#include "decl/lib/jvm/mh/setter.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_setter() {
	mh_setter_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"jvm/mh/Setter" }
	);

	mh_setter_constructor = mh_setter_class->declared_instance_methods().find(
		c_string{ u8"<init>" },
		c_string{ u8"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;S)V" }
	);

	mh_setter_class->declared_instance_methods().find(
		c_string{ u8"invokeExactPtr" }, c_string{ u8"()V" }
	).native_function(
		(void*)+[](
			j::method_handle& ths0
		) -> optional<reference> {
			jvm::class_member& ths = (jvm::class_member&) ths0;

			instance_field& resolved_field
				= ths.member<instance_field_index>();

			optional<reference> possible_throwable
				= try_put_field_resolved(resolved_field);
			
			if(possible_throwable.has_value()) {
				return possible_throwable.move();
			}

			return {};
		}
	);
}