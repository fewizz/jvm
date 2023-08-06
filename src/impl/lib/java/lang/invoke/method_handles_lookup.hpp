#include "decl/lib/java/lang/invoke/method_handles_lookup.hpp"

#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/lib/jvm/mh/getter.hpp"
#include "decl/lib/jvm/mh/setter.hpp"
#include "decl/lib/jvm/mh/constructor.hpp"
#include "decl/lib/jvm/mh/static.hpp"
#include "decl/lib/jvm/mh/virtual.hpp"
#include "decl/lib/jvm/mh/special.hpp"
#include "decl/classes.hpp"
#include "decl/class/resolve_method.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/execute.hpp"

#include <range.hpp>
#include <span.hpp>

static expected<reference, reference> try_lookup_find_getter(
	j::c& c_inst,
	j::string& name,
	j::c& field_type_inst
) {
	::c& c = c_inst.get_c();
	::c& field_c = field_type_inst.get_c();

	instance_field_index index =
		name.view_on_stack_as_utf8([&](auto name_utf8)  {
			return c.instance_fields().find_index_of(
				name_utf8, field_c.descriptor()
			);
		});

	expected<reference, reference> possible_mt
		= try_create_method_type(field_c, span{&c});

	if(possible_mt.is_unexpected()) {
		return unexpected{ possible_mt.move_unexpected() };
	}

	reference mt = possible_mt.move_expected();

	return try_create_getter_mh(move(mt), c, index);
}

static expected<reference, reference> try_lookup_find_setter(
	j::c& c_inst,
	j::string& name,
	j::c& field_type_inst
) {
	::c& c = c_inst.get_c();
	::c& field_c = field_type_inst.get_c();

	instance_field_index index =
		name.view_on_stack_as_utf8([&](auto name_utf8) {
			return c.instance_fields().find_index_of(
				name_utf8, field_c.descriptor()
			);
		});

	array arg_types{ &c, &field_c };

	expected<reference, reference> possible_mt
		= try_create_method_type(
			void_class.get(), arg_types.dereference_view()
		);

	if(possible_mt.is_unexpected()) {
		return unexpected{ possible_mt.move_unexpected() };
	}

	reference mt = possible_mt.move_expected();

	return try_create_setter_mh(move(mt), c, index);
}

static expected<reference, reference> try_lookup_find_virtual(
	j::c& c_inst,
	j::string& name,
	j::method_type& mt
) {
	::c& c = c_inst.get_c();

	method& m = name.view_on_stack_as_utf8([&](auto name_utf8) -> method& {
		return
			c.instance_methods().find(
				name_utf8,
				mt.descriptor()
			);
	});

	return try_create_virtual_mh(mt, m);
}

static expected<reference, reference>
try_lookup_find_static(
	j::c& cls,
	j::string& name,
	j::method_type& mt
) {
	c& c = cls.get_c();

	method& m = name.view_on_stack_as_utf8([&](auto name_utf8) -> method& {
		return c
			.declared_static_methods().find(
				name_utf8,
				mt.descriptor()
			);
		}
	);

	return try_create_static_mh(mt, m);
}

static expected<reference, reference> try_lookup_find_special(
	j::c& refc,
	j::string& name,
	j::method_type& mt,
	j::c& special_caller
) {
	return name.view_on_stack_as_utf8([&](auto name_utf8)
	-> expected<reference, reference>
	{
		if(name_utf8.has_equal_size_and_elements(c_string{ u8"<init>" })) {
			posix::abort(); // TODO throw NoSuchElementException
		}
		::c& c = refc.get_c();
		::c& d = special_caller.get_c();
		expected<method&, reference> possible_resolved_method =
			try_resolve_method(
				d, c, name_utf8, mt.descriptor()
			);

		if(possible_resolved_method.is_unexpected()) {
			return unexpected {
				possible_resolved_method.move_unexpected()
			};
		}

		method& resolved_method = possible_resolved_method.get_expected();

		optional<instance_method&> possible_selected_method
			= select_method_for_invoke_special(
				d, c, resolved_method
			);

		if(!possible_selected_method.has_value()) {
			posix::abort();
		}

		instance_method& selected_method = possible_selected_method.get();

		return try_create_special_mh(
			mt, selected_method
		);
	});
}

static expected<reference, reference> try_lookup_find_constructor(
	j::c& refc, j::method_type& mt
) {
	c& c = refc.get_c();
	method& m
		= c.declared_instance_methods().find(
			c_string{ u8"<init>" }, mt.descriptor()
		);
	return try_create_constructor_mh(mt, m);
}

static void init_java_lang_invoke_method_handles_lookup() {
	method_handles_lookup_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/invoke/MethodHandles$Lookup" }
	);

	method_handles_lookup_class->declared_instance_methods().find(
		c_string{ u8"findGetter" },
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
			native_environment*,
			object*,
			j::c* cls,
			j::string* name,
			j::c* c
		) -> object* {
			if(cls == nullptr || name == nullptr || c == nullptr) {
				thrown_in_native = try_create_null_pointer_exception().get();
				return nullptr;
			}
			expected<reference, reference> possible_mh
				= try_lookup_find_getter(*cls, *name, *c);
			if(possible_mh.is_unexpected()) {
				thrown_in_native = possible_mh.move_unexpected();
				return nullptr;
			}
			reference mh = possible_mh.move_expected();
			return & mh.unsafe_release_without_destroing();
		}
	);

	method_handles_lookup_class->declared_instance_methods().find(
		c_string{ u8"findSetter" },
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
			native_environment*,
			object*,
			j::c* cls,
			j::string* name,
			j::c* c
		) -> object* {
			if(cls == nullptr || name == nullptr || c == nullptr) {
				thrown_in_native = try_create_null_pointer_exception().get();
				return nullptr;
			}
			expected<reference, reference> possible_mh
				= try_lookup_find_setter(*cls, *name, *c);
			if(possible_mh.is_unexpected()) {
				thrown_in_native = possible_mh.move_unexpected();
				return nullptr;
			}
			reference mh = possible_mh.move_expected();
			return & mh.unsafe_release_without_destroing();
		}
	);

	method_handles_lookup_class->declared_instance_methods().find(
		c_string{ u8"findVirtual" },
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
			native_environment*,
			object*,
			j::c* cls,
			j::string* name,
			j::method_type* mt
		) -> object* {
			if(cls == nullptr || name == nullptr || mt == nullptr) {
				thrown_in_native = try_create_null_pointer_exception().get();
				return nullptr;
			}

			expected<reference, reference> possible_mh
				= try_lookup_find_virtual(*cls, *name, *mt);

			if(possible_mh.is_unexpected()) {
				thrown_in_native = possible_mh.move_unexpected();
				return nullptr;
			}
			reference mh = possible_mh.move_expected();
			return & mh.unsafe_release_without_destroing();
		}
	);

	method_handles_lookup_class->declared_instance_methods().find(
		c_string{ u8"findStatic" },
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
			native_environment*,
			object*,
			j::c* cls,
			j::string* name,
			j::method_type* mt
		) -> object* {
			if(cls == nullptr || name == nullptr || mt == nullptr) {
				thrown_in_native = try_create_null_pointer_exception().get();
				return nullptr;
			}

			expected<reference, reference> possible_mh
				= try_lookup_find_static(*cls, *name, *mt);

			if(possible_mh.is_unexpected()) {
				thrown_in_native = possible_mh.move_unexpected();
				return nullptr;
			}
			reference mh = possible_mh.move_expected();
			return & mh.unsafe_release_without_destroing();
		}
	);

	method_handles_lookup_class->declared_instance_methods().find(
		c_string{ u8"findConstructor" },
		c_string {
			"("
				"Ljava/lang/Class;"
				"Ljava/lang/invoke/MethodType;"
			")"
			"Ljava/lang/invoke/MethodHandle;"
		}
	).native_function(
		(void*)+[](
			native_environment*,
			object*,
			j::c* cls,
			j::method_type* mt
		) -> object* {
			if(cls == nullptr || mt == nullptr) {
				thrown_in_native = try_create_null_pointer_exception().get();
				return nullptr;
			}

			expected<reference, reference> possible_mh
				= try_lookup_find_constructor(*cls, *mt);

			if(possible_mh.is_unexpected()) {
				thrown_in_native = possible_mh.move_unexpected();
				return nullptr;
			}
			reference mh = possible_mh.move_expected();
			return & mh.unsafe_release_without_destroing();
		}
	);

	method_handles_lookup_class->declared_instance_methods().find(
		c_string{ u8"findSpecial" },
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
			native_environment*,
			object*,
			j::c* cls,
			j::string* name,
			j::method_type* mt,
			j::c* caller
		) -> object* {
			if(
				cls == nullptr || name   == nullptr ||
				mt  == nullptr || caller == nullptr
			) {
				thrown_in_native = try_create_null_pointer_exception().get();
				return nullptr;
			}

			expected<reference, reference> possible_mh
				= try_lookup_find_special(*cls, *name, *mt, *caller);
			if(possible_mh.is_unexpected()) {
				thrown_in_native = possible_mh.move_unexpected();
				return nullptr;
			}
			reference mh = possible_mh.move_expected();
			return & mh.unsafe_release_without_destroing();
		}
	);
}