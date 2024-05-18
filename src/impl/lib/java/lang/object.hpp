#include "decl/lib/java/lang/object.hpp"

#include "decl/class.hpp"
#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/native/environment.hpp"

static inline void init_java_lang_object() {
	object_class = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/Object"sv
	);

	object_class->declared_instance_methods()
	.find(u8"hashCode"sv, u8"()I"sv)
	.native_function(
		(void*)+[](native_environment*, object* o) {
			return (int32) (nuint) o;
		}
	);

	object_class->declared_instance_methods()
	.find(u8"getClass"sv, u8"()Ljava/lang/Class;"sv)
	.native_function(
		(void*)+[](native_environment*, object* o) -> ::object* {
			return o->c().object_ptr();
		}
	);

}