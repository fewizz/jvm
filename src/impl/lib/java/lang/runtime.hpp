#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/execution/context.hpp"
#include "decl/execution/thread.hpp"

extern "C" [[ noreturn ]] void exit(int exit_code);

static inline void init_java_lang_runtime() {
	c& runtime_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/Runtime" }
	);

	runtime_class.declared_instance_methods().find(
		c_string{ u8"exit" }, c_string{ u8"(I)V" }
	).native_function(
		(void*)+[](native_environment*, object*, int32 status) {
			thrown_in_native = reference{};
			thread = reference{};
			stack.erase_back_until(0);
			exit(status);
		}
	);

	runtime_class.declared_instance_methods().find(
		c_string{ u8"availableProcessors" }, c_string{ u8"()I" }
	).native_function(
		(void*)+[](native_environment*, object*) {
			return 1; // TODO
		}
	);

}