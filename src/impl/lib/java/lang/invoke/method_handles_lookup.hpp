#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"
#include "decl/classes.hpp"
#include "decl/native/interface/environment.hpp"

static reference lookup_find_static(object& cls, object& name, object& mt) {
	return view_string_on_stack_as_utf8(name, [&](auto name_utf8) {
		method& m =
			class_from_class_instance(cls)
			.declared_static_methods().find(
				name_utf8,
				method_type_descriptor(mt)
			);
		
		return create_method_handle_invoke_static(m);
	});
}

static void init_java_lang_invoke_method_handles_lookup() {
	_class& method_handles_lookup_class = classes.find_or_load(
		c_string{ "java/lang/invoke/MethodHandles$Lookup" }
	);

	method_handles_lookup_class.declared_methods().find(
		c_string{ "findStatic" },
		c_string {
			"("
				"Ljava/lang/Class;"
				"Ljava/lang/String;"
				"Ljava/lang/invoke/MethodType;"
			")"
			"Ljava/lang/invoke/MethodHandle;"
		}
	).native_function(
		(void*)
		(object*(*)(
			native_interface_environment*, object*, object*, object*, object*
		))
		[](
			native_interface_environment*, object*,
			object* cls, object* name, object* mt
		) -> object* {
			return &
				lookup_find_static(*cls, *name, *mt)
				.unsafe_release_without_destroing();
		}
	);
}