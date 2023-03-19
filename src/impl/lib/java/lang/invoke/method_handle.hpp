#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "decl/method.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/object.hpp"

#include <class_file/constant.hpp>

inline void init_java_lang_invoke_method_handle() {
	method_handle_class = classes.find_or_load(
		c_string{ "java/lang/invoke/MethodHandle" }
	);

	method_handle_function_ptr_position
		= method_handle_class->instance_field_position(
			c_string{"functionPtr_"}, c_string{"J"}
		);
}

inline void method_handle_invoke_exact(reference mh_ref, nuint args_beginning) {
	uint64 ptr0 = mh_ref->get<int64>(
		method_handle_function_ptr_position
	);
	using f = void(*)(reference mh_ref, nuint args_beginning);
	((f)ptr0)(move(mh_ref), args_beginning);
}