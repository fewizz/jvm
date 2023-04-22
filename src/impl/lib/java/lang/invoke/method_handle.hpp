#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "decl/method.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/object.hpp"

#include <class_file/constant.hpp>

inline void init_java_lang_invoke_method_handle() {
	method_handle_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/invoke/MethodHandle" }
	);

	method_handle_invoke_exact_ptr_index
		= method_handle_class->instance_methods().find_index_of(
			c_string{"invokeExactPtr"}, c_string{"()V"}
		);
	
	method_handle_method_type_field_position
		= method_handle_class->instance_field_position(
			c_string{"methodType_"}, c_string{"Ljava/lang/invoke/MethodType;"}
		);
}

[[nodiscard]] inline optional<reference>
method_handle_invoke_exact(reference mh_ref, nuint args_beginning) {
	method& m = mh_ref->_class().instance_methods()
		[method_handle_invoke_exact_ptr_index];

	void* ptr0 = m.native_function();
	using f = optional<reference>(*)(reference mh_ref, nuint args_beginning);
	return ((f)ptr0)(move(mh_ref), args_beginning);
}