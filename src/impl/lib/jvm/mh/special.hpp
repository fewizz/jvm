#include "decl/lib/jvm/mh/special.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/execute.hpp"
#include "decl/thrown.hpp"

static void init_jvm_mh_special() {
	mh_special_class = classes.find_or_load(c_string{"jvm/mh/Special"});

	mh_special_constructor = mh_special_class->declared_instance_methods().find(
		c_string{"<init>"},
		c_string{"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;I)V"}
	);

	mh_special_class->declared_instance_methods().find(
		c_string{"invokeExactPtr"}, c_string{"()V"}
	).native_function(
		(void*)+[](
			reference mh,
			nuint args_beginning
		) -> void {
			uint32 index = mh->get<int32>(mh_class_member_index_position);

			reference& refc_ref
				= mh->get<reference>(mh_class_member_class_position);
			_class& refc = class_from_class_instance(refc_ref);

			reference obj_ref = stack.get<reference>(args_beginning);

			bool valid =
				obj_ref._class().is(refc) ||
				obj_ref._class().is_sub_of(refc);

			if(!valid) {
				thrown = create_wrong_method_type_exception();
			}

			execute(refc.instance_methods()[index]);
		}
	);
}