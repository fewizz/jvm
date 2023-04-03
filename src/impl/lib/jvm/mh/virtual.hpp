#include "decl/lib/jvm/mh/virtual.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/execute.hpp"
#include "decl/thrown.hpp"

static void init_jvm_mh_virtual() {
	mh_virtual_class = classes.load_class_by_bootstrap_class_loader(
		c_string{"jvm/mh/Virtual"}
	);

	mh_virtual_constructor = mh_virtual_class->declared_instance_methods().find(
		c_string{"<init>"},
		c_string{"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;S)V"}
	);

	mh_virtual_class->declared_instance_methods().find(
		c_string{"invokeExactPtr"}, c_string{"()V"}
	).native_function(
		(void*)+[](reference mh, nuint args_beginning) -> void {
			instance_method_index method_index {
				mh->get<uint16>(mh_class_member_index_position)
			};
			reference& refc_ref
				= mh->get<reference>(mh_class_member_class_position);

			_class& refc = class_from_class_instance(refc_ref);

			reference& obj_ref = stack.get<reference>(args_beginning);

			bool valid =
				obj_ref->_class().is(refc) ||
				obj_ref->_class().is_sub_of(refc);

			if(!valid) {
				thrown = create_wrong_method_type_exception();
				return;
			}

			execute(obj_ref._class()[method_index]);
		}
	);
}