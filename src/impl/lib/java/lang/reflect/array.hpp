#include "decl/array.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/negative_array_size_exception.hpp"

static void init_java_lang_reflect_array() {
	classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/reflect/Array" }
	).declared_methods().find(
		c_string{ "newInstance" },
		c_string{ "(Ljava/lang/Class;I)Ljava/lang/Object;" }
	).native_function(
		(void*)+[](
			native_environment*, object* component_type, int32 len
		) -> object* {
			if(len < 0) {
				expected<reference, reference> possible_nase
					= try_create_negative_array_size_exception();
				thrown_in_native = move(possible_nase.get());
				return nullptr;
			}
			_class& c = class_from_class_instance(*component_type);
			expected<reference, reference> possible_array
				= try_create_array_of(c, len);
			
			if(possible_array.is_unexpected()) {
				thrown_in_native = move(possible_array.get_unexpected());
				return nullptr;
			}

			reference array = move(possible_array.get_expected());

			return & array.unsafe_release_without_destroing();
		}
	);

}