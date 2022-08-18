#include "decl/array.hpp"
#include "decl/classes.hpp"
#include "decl/thrown.hpp"
#include "decl/native/interface/environment.hpp"
#include "decl/lib/java/lang/negative_array_size_exception.hpp"

static void init_java_lang_reflect_array() {
	classes.find_or_load(c_string{ "java/lang/reflect/Array" })
	.declared_methods().find(
		c_string{ "newInstance" },
		c_string{ "(Ljava/lang/Class;I)Ljava/lang/Object;" }
	).native_function(
		(void*) (object*(*)(native_interface_environment*, object*, int32))
		[](
			native_interface_environment*, object* component_type, int32 len
		) -> object* {
			if(len < 0) {
				thrown = create_negative_array_size_exception();
				return nullptr;
			}
			_class& c = class_from_class_instance(*component_type);
			reference array = create_array_of(c, len);
			return & array.unsafe_release_without_destroing();
		}
	);

}