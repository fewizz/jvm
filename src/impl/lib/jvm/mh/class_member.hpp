#include "decl/lib/jvm/mh/class_member.hpp"

#include "classes.hpp"

static void init_jvm_mh_class_member() {
	mh_class_member_class = classes.find_or_load(
		c_string{"jvm/mh/ClassMember"}
	);

	mh_class_member_class_position
		= mh_class_member_class->instance_field_position(
			c_string{"class_"}, c_string{"Ljava/lang/Class;"}
		);

	mh_class_member_index_position
		= mh_class_member_class->instance_field_position(
			c_string{"memberIndex_"}, c_string{"S"}
		);
}