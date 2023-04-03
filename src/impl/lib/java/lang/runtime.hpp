#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/execution/context.hpp"
#include "decl/execution/thread.hpp"
#include "decl/thrown.hpp"

extern "C" [[ noreturn ]] void exit(int exit_code);

static inline void init_java_lang_runtime() {
	_class& runtime_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Runtime" }
	);

	runtime_class.declared_methods().find(
		c_string{ "exit" }, c_string{ "(I)V" }
	).native_function(
		(void*)+[](native_environment*, object*, int32 status) {
			thrown = reference{};
			thread = reference{};
			stack.pop_back_until(0);
			exit(status);
		}
	);

	runtime_class.declared_methods().find(
		c_string{ "availableProcessors" }, c_string{ "()I" }
	).native_function(
		(void*)+[](native_environment*, object*) {
			return 1; // TODO
		}
	);

}