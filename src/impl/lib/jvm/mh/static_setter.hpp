#include "decl/lib/jvm/mh/static_setter.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_static_setter() {
	mh_static_setter_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"jvm/mh/StaticSetter" }
	);

	mh_static_setter_constructor
		= mh_static_setter_class->declared_instance_methods().find(
			c_string{ u8"<init>" },
			c_string{ u8"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;S)V" }
		);

	mh_static_setter_class->declared_instance_methods().find(
		c_string{ u8"invokeExactPtr" }, c_string{ u8"()V" }
	).native_function(
		(void*)+[](
			j::method_handle& mh
		) -> optional<reference> {

			reference& c_ref
				= mh.get<reference>(mh_class_member_class_position);
			c& c = class_from_class_instance(c_ref);

			optional<reference> optional_throwable
				= c.try_initialise_if_need();//TODO
			if(optional_throwable.has_value()) {
				return move(optional_throwable.get());
			}

			declared_static_field_index index {
				mh.get<uint16>(mh_class_member_index_position)
			};

			static_field& resolved_field = c[index];

			put_static_resolved(resolved_field);

			return {};
		}
	);
}