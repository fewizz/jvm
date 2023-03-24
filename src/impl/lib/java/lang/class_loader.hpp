#include "decl/lib/java/lang/class_loader.hpp"

#include "decl/lib/java/lang/string.hpp"
#include "decl/lib/java/lang/index_out_of_bounds_exception.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/thrown.hpp"

#include "classes.hpp"

static void init_java_lang_class_loader() {
	_class& c = classes.find_or_load(c_string{"java/lang/ClassLoader"});
	class_loader_load_class_method_index
		= c.instance_methods().find_index_of(
			c_string{"loadClass"},
			c_string{"(Ljava/lang/String;)Ljava/lang/Class;"}
		);

	c.declared_instance_methods().find(
		c_string{"defineClass"},
		c_string{"(Ljava/lang/String;[BII)Ljava/lang/Class;"}
	).native_function((void*)+[](object* ths, object* b, int32 off, int32 len) {
		if(b == nullptr) {
			thrown = create_null_pointer_exception();
			return;
		}

		span<uint8> bytes = array_as_span<uint8>(*b);

		if(off < 0 || len < 0 || (uint32)(off + len) > bytes.size()) {
			thrown = create_index_of_of_bounds_exception();
			return;
		}

		auto data = posix::allocate_memory_for<uint8>(bytes.size());
		bytes.copy_to(data.as_span());

		define_class(move(data), reference{ *ths });
	});

	c.declared_static_methods().find(
		c_string{ "loadClassJVM" },
		c_string{ "(Ljava/lang/String;)Ljava/lang/Class;" }
	).native_function((void*)+[](object* name) -> object* {
		_class& c = view_string_on_stack_as_utf8(
			*name,
			[](auto name_utf8) -> _class& {
				for(char& cp : name_utf8) {
					if(cp == '.') cp = '/';
				}
				return load_class(name_utf8);
			}
		);
		return & c.instance().unsafe_release_without_destroing();
	});

	c.declared_instance_methods().find(
		c_string{ "findLoadedClass" },
		c_string{ "(Ljava/lang/String;)Ljava/lang/Class;" }
	).native_function(
		// TODO
		//classes.try_find(Name &&name)
	);
}