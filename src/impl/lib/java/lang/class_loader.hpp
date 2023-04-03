#include "decl/lib/java/lang/class_loader.hpp"

#include "decl/lib/java/lang/string.hpp"
#include "decl/lib/java/lang/index_out_of_bounds_exception.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/thrown.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include "classes.hpp"

static void init_java_lang_class_loader() {
	_class& c = classes.load_class_by_bootstrap_class_loader(
		c_string{"java/lang/ClassLoader"}
	);

	class_loader_load_class_method_index
		= c.instance_methods().find_index_of(
			c_string{"loadClass"},
			c_string{"(Ljava/lang/String;)Ljava/lang/Class;"}
		);

	c.declared_instance_methods().find(
		c_string{"defineClass"},
		c_string{"(Ljava/lang/String;[BII)Ljava/lang/Class;"}
	).native_function((void*)+[](
		native_environment*,
		object* ths, object* name, object* b, int32 off, int32 len
	)
	-> object*
	{
		if(b == nullptr) {
			thrown = create_null_pointer_exception();
			return nullptr;
		}

		span<uint8> bytes = array_as_span<uint8>(*b);

		if(off < 0 || len < 0 || (uint32)(off + len) > bytes.size()) {
			thrown = create_index_of_of_bounds_exception();
			return nullptr;
		}

		auto data = posix::allocate_memory_for<uint8>(bytes.size());
		bytes.copy_to(data.as_span());

		_class& c = view_string_on_stack_as_utf8(
			*name,
			[&](auto name_utf8) -> _class& {
				return classes.define_class(name_utf8, move(data), *ths);
			}
		);

		return & c.instance().unsafe_release_without_destroing();
	});

	c.declared_static_methods().find(
		c_string{ "loadClassJVM" },
		c_string{ "(Ljava/lang/String;)Ljava/lang/Class;" }
	).native_function((void*)+[](native_environment*, object* name) -> object* {
		_class& c = view_string_on_stack_as_utf8(
			*name,
			[](auto name_utf8) -> _class& {
				for(char& cp : name_utf8) {
					if(cp == '.') cp = '/';
				}
				return classes.load_class_by_bootstrap_class_loader(name_utf8);
			}
		);
		return & c.instance().unsafe_release_without_destroing();
	});

	c.declared_instance_methods().find(
		c_string{ "findLoadedClass" },
		c_string{ "(Ljava/lang/String;)Ljava/lang/Class;" }
	).native_function((void*)+[](
		native_environment*, object* ths, object* name
	) -> object* {
		optional<_class&> possible_c
		= view_string_on_stack_as_utf8(
			*name,
			[&](auto name_utf8) -> optional<_class&> {
				for(char& cp : name_utf8) {
					if(cp == '.') cp = '/';
				}

				return classes.try_find_class_which_loading_was_initiated_by(
					name_utf8,
					*ths
				);
			}
		);
		if(possible_c.has_no_value()) {
			return nullptr;
		}

		return & possible_c->instance()
			.unsafe_release_without_destroing();
	});
}