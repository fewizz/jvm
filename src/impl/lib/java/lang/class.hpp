#include "decl/lib/java/lang/class.hpp"

#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/string.hpp"

#include <c_string.hpp>

static inline c& class_from_class_instance(object& class_instance) {
	return * (c*) class_instance.get<int64>(class_ptr_field_position);
}

static inline void init_java_lang_class() {
	class_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Class" }
	);
	class_ptr_field_position = class_class->instance_field_position(
		c_string{ "ptr_" }, c_string{ "J" }
	);

	class_class->declared_instance_methods().find(
		c_string{ "getComponentType" }, c_string{ "()Ljava/lang/Class;" }
	).native_function(
		(void*)+[](native_environment*, object* ths) -> object* {
			c& c = class_from_class_instance(*ths);
			return c.get_component_class().object_ptr();
		}
	);

	class_class->declared_instance_methods().find(
		c_string{ "getName" }, c_string{ "()Ljava/lang/String;" }
	).native_function(
		(void*)+[](native_environment*, object* ths) -> object* {
			c& c = class_from_class_instance(*ths);
			expected<reference, reference> possible_string
				= try_create_string_from_utf8(c.name());
			if(possible_string.is_unexpected()) {
				thrown_in_native = possible_string.move_unexpected();
				return nullptr;
			}
			reference string = possible_string.move_expected();
			return & string.unsafe_release_without_destroing();
		}
	);
}