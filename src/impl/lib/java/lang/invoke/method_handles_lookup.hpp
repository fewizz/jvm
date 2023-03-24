#include "decl/lib/java/lang/invoke/method_handles_lookup.hpp"

#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"
#include "decl/lib/jvm/mh/getter.hpp"
#include "decl/lib/jvm/mh/constructor.hpp"
#include "decl/lib/jvm/mh/static.hpp"
#include "decl/lib/jvm/mh/virtual.hpp"
#include "decl/lib/jvm/mh/special.hpp"
#include "decl/classes.hpp"
#include "decl/class/resolve_method.hpp"
#include "decl/native/environment.hpp"
#include "decl/execute.hpp"

#include <range.hpp>
#include <span.hpp>

static reference lookup_find_getter(
	object& c_inst, object& name, object& field_type_inst
) {
	_class& c = class_from_class_instance(c_inst);
	_class& field_c = class_from_class_instance(field_type_inst);

	return view_string_on_stack_as_utf8(name, [&](auto name_utf8) {
		instance_field_index index
			= c.instance_fields().find_index_of(
				name_utf8, field_c.descriptor()
			);

		reference method_type = create_method_type(field_c, span<_class&>{});
		
		reference result = create_object(mh_getter_class.get());
		execute(
			mh_getter_constructor.get(),
			result, // this
			method_type, c_inst, uint16{ index }
		);
		return result;
	});
}

static reference lookup_find_virtual(
	object& c_inst, object& name, object& mt
) {
	return view_string_on_stack_as_utf8(name, [&](auto name_utf8) {
		instance_method_index index =
			class_from_class_instance(c_inst)
			.instance_methods().find_index_of(
				name_utf8,
				method_type_descriptor(mt)
			);

		reference result = create_object(mh_virtual_class.get());
		execute(
			mh_virtual_constructor.get(),
			result, // this
			mt, c_inst, uint16{ index }
		);
		return result;
	});
}

static reference lookup_find_static(object& cls, object& name, object& mt) {
	return view_string_on_stack_as_utf8(name, [&](auto name_utf8) {
		declared_static_method_index index =
			class_from_class_instance(cls)
			.declared_static_methods().find_index_of(
				name_utf8,
				method_type_descriptor(mt)
			);
		
		reference result = create_object(mh_static_class.get());
		execute(
			mh_static_constructor.get(),
			result, // this
			mt, cls, uint16{ index }
		);
		return result;
	});
}

static reference lookup_find_special(
	object& refc, object& name, object& mt, object& special_caller
) {
	return view_string_on_stack_as_utf8(name, [&](auto name_utf8) -> reference {
		if(name_utf8.has_equal_size_and_elements(c_string{ "<init>" })) {
			posix::abort(); // TODO throw NoSuchElementException
		}
		_class& receiver = class_from_class_instance(refc);
		_class& current = class_from_class_instance(special_caller);
		method& resolved_method =
			::resolve_method(receiver, name_utf8, method_type_descriptor(mt));
		method& m = select_method_for_invoke_special(
			current, receiver, resolved_method
		);

		optional<instance_method_index> possible_index
			= m._class().instance_methods()
				.try_find_index_of_first_satisfying([&](method& m0) {
					return &m0 == &m;
				});

		possible_index.if_has_no_value(posix::abort);

		reference result = create_object(mh_special_class.get());
		execute(
			mh_special_constructor.get(),
			result, // this
			mt, m._class().instance(), uint16{ possible_index.get() }
		);
		return result;
	});
}

static reference lookup_find_constructor(
	object& refc, object& mt
) {
	_class& c = class_from_class_instance(refc);
	declared_instance_method_index index
		= c.declared_instance_methods().find_index_of(
			c_string{ "<init>" }, method_type_descriptor(mt)
		);
	reference result = create_object(mh_constructor_class.get());
	execute(
		mh_constructor_constructor.get(),
		result, // this,
		mt, refc, uint16 { index }
	);
	return result;
}

static void init_java_lang_invoke_method_handles_lookup() {
	method_handles_lookup_class = classes.find_or_load(
		c_string{ "java/lang/invoke/MethodHandles$Lookup" }
	);

	method_handles_lookup_class->declared_methods().find(
		c_string{ "findGetter" },
		c_string {
			"("
				"Ljava/lang/Class;"
				"Ljava/lang/String;"
				"Ljava/lang/Class;"
			")"
			"Ljava/lang/invoke/MethodHandle;"
		}
	).native_function(
		(void*)+[](
			native_environment*, object*,
			object* cls, object* name, object* c
		) -> object* {
			return &
				lookup_find_getter(*cls, *name, *c)
				.unsafe_release_without_destroing();
		}
	);

	method_handles_lookup_class->declared_methods().find(
		c_string{ "findVirtual" },
		c_string {
			"("
				"Ljava/lang/Class;"
				"Ljava/lang/String;"
				"Ljava/lang/invoke/MethodType;"
			")"
			"Ljava/lang/invoke/MethodHandle;"
		}
	).native_function(
		(void*)+[](
			native_environment*, object*,
			object* cls, object* name, object* mt
		) -> object* {
			return &
				lookup_find_virtual(*cls, *name, *mt)
				.unsafe_release_without_destroing();
		}
	);

	method_handles_lookup_class->declared_methods().find(
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
		(void*)+[](
			native_environment*, object*,
			object* cls, object* name, object* mt
		) -> object* {
			return &
				lookup_find_static(*cls, *name, *mt)
				.unsafe_release_without_destroing();
		}
	);

	method_handles_lookup_class->declared_methods().find(
		c_string{ "findConstructor" },
		c_string {
			"("
				"Ljava/lang/Class;"
				"Ljava/lang/invoke/MethodType;"
			")"
			"Ljava/lang/invoke/MethodHandle;"
		}
	).native_function(
		(void*)+[](
			native_environment*, object*,
			object* cls, object* mt
		) -> object* {
			return &
				lookup_find_constructor(*cls, *mt)
				.unsafe_release_without_destroing();
		}
	);

	method_handles_lookup_class->declared_methods().find(
		c_string{ "findSpecial" },
		c_string {
			"("
				"Ljava/lang/Class;"
				"Ljava/lang/String;"
				"Ljava/lang/invoke/MethodType;"
				"Ljava/lang/Class;"
			")"
			"Ljava/lang/invoke/MethodHandle;"
		}
	).native_function(
		(void*)+[](
			native_environment*, object*,
			object* cls, object* name, object* mt, object* caller
		) -> object* {
			return &
				lookup_find_special(*cls, *name, *mt, *caller)
				.unsafe_release_without_destroing();
		}
	);
}