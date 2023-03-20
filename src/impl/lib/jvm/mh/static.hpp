#include "decl/lib/jvm/mh/static.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_static() {
	mh_static_class = classes.find_or_load(c_string{"jvm/mh/Static"});

	mh_static_constructor = mh_static_class->declared_instance_methods().find(
		c_string{"<init>"},
		c_string{"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;I)V"}
	);

	mh_static_class->declared_instance_methods().find(
		c_string{"invokeExactPtr"}, c_string{"()V"}
	).native_function(
		(void*)+[](
			reference ths,
			[[maybe_unused]] nuint args_beginning
		) -> void {
			uint32 index = ths->get<int32>(mh_class_member_index_position);
			reference& class_ref
				= ths->get<reference>(mh_class_member_class_position);

			_class& c = class_from_class_instance(class_ref);

			execute(c.declared_static_methods()[index]);
		}
	);
}