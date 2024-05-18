#include "decl/lib/jvm/mh/getter.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_getter() {
	mh_getter_class = classes.load_class_by_bootstrap_class_loader(
		u8"jvm/mh/Getter"sv
	);

	mh_getter_constructor = mh_getter_class->declared_instance_methods().find(
		u8"<init>"sv,
		u8"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;S)V"sv
	);

	mh_getter_class->declared_instance_methods().find(
		u8"invokeExactPtr"sv, u8"()V"sv
	).native_function(
		(void*)+[](
			j::method_handle& ths0
		) -> optional<reference> {
			jvm::class_member& ths = (jvm::class_member&) ths0;

			instance_field& resolved_field
				= ths.member<instance_field_index>();

			optional<reference> possible_throwable
				= try_get_field_resolved(resolved_field);
			
			if(possible_throwable.has_value()) {
				return possible_throwable.move();
			}

			return {};
		}
	);
}