#include "decl/lib/java/lang/class.hpp"

#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <c_string.hpp>

static inline _class& class_from_class_instance(object& class_instance) {
	return * (_class*) class_instance.get<int64>(class_ptr_field_position);
}

static inline void init_java_lang_class() {
	class_class = classes.find_or_load(c_string{ "java/lang/Class" });
	class_ptr_field_position = class_class->instance_field_position(
		c_string{ "ptr_" }, c_string{ "J" }
	);

	class_class->declared_methods().find(
		c_string{ "getComponentType" }, c_string{ "()Ljava/lang/Class;" }
	).native_function(
		(void*) (object*(*)(native_environment*, object*))
		[](native_environment*, object* ths) -> object* {
			_class& c = class_from_class_instance(*ths);
			return c.get_component_class().instance().object_ptr();
		}
	);
}