#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

extern "C" [[ noreturn ]] void exit(int exit_code);

static inline void init_java_lang_runtime() {
	_class& runtime_class = classes.find_or_load(
		c_string{ "java/lang/Runtime" }
	);

	runtime_class.declared_methods().find(
		c_string{ "exit" }, c_string{ "(I)V" }
	).native_function(
		(void*) (void(*)(native_environment*, object*, int32))
		[](native_environment*, object*, int32 status) {
			exit(status);
		}
	);

	runtime_class.declared_methods().find(
		c_string{ "availableProcessors" }, c_string{ "()I" }
	).native_function(
		(void*) (int32(*)(native_environment*, object*))
		[](native_environment*, object*) {
			return 1; // TODO
		}
	);

}