#include "decl/lib/jvm/mh/setter.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_setter() {
	mh_setter_class = classes.load_class_by_bootstrap_class_loader(
		c_string{"jvm/mh/Setter"}
	);

	mh_setter_constructor = mh_setter_class->declared_instance_methods().find(
		c_string{"<init>"},
		c_string{"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;S)V"}
	);

	mh_setter_class->declared_instance_methods().find(
		c_string{"invokeExactPtr"}, c_string{"()V"}
	).native_function(
		(void*)+[](
			reference mh,
			[[maybe_unused]] nuint args_beginning
		) -> optional<reference> {
			reference& c_ref
				= mh->get<reference>(mh_class_member_class_position);
			_class& c = class_from_class_instance(c_ref);

			instance_field_index index {
				mh->get<uint16>(mh_class_member_index_position)
			};

			field& resolved_field = c[index];

			optional<reference> possible_throwable
				= try_put_field_resolved(resolved_field);
			
			if(possible_throwable.has_value()) {
				return move(possible_throwable.get());
			}

			return {};
		}
	);
}