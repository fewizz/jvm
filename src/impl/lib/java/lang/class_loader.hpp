#include "decl/lib/java/lang/class_loader.hpp"

#include "decl/lib/java/lang/string.hpp"
#include "decl/lib/java/lang/index_out_of_bounds_exception.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"

#include "classes.hpp"

static void init_java_lang_class_loader() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/ClassLoader" }
	);

	class_loader_load_class_method_index
		= c.instance_methods().find_index_of(
			c_string{ u8"loadClass" },
			c_string{ u8"(Ljava/lang/String;)Ljava/lang/Class;" }
		);

	c.declared_instance_methods().find(
		c_string{ u8"defineClass" },
		c_string{ u8"(Ljava/lang/String;[BII)Ljava/lang/Class;" }
	).native_function((void*)+[](
		native_environment*,
		o<jl::c_loader>* ths,
		o<jl::string>* name,
		o<jl::object>* b,
		int32 off,
		int32 len
	)
	-> o<jl::object>*
	{
		if(b == nullptr) {
			expected<reference, reference> possible_npe
				= try_create_null_pointer_exception();
			thrown_in_native = possible_npe.move();
			return nullptr;
		}

		span<uint8> bytes = array_as_span<uint8>(*b);

		if(off < 0 || len < 0 || (uint32)(off + len) > bytes.size()) {
			thrown_in_native = try_create_index_of_of_bounds_exception().get();
			return nullptr;
		}

		auto data = posix::allocate<>(bytes.size());
		bytes.copy_to(data.as_span());

		expected<::c&, reference> possible_c = name->view_on_stack_as_utf8(
			[&](span<utf8::unit> name_utf8) -> expected<::c&, reference> {
				return classes.try_define_class(name_utf8, move(data), ths);
			}
		);

		if(possible_c.is_unexpected()) {
			thrown_in_native = possible_c.move_unexpected();
			return nullptr;
		}

		::c& c = possible_c.get_expected();

		return c.object_ptr();
	});

	c.declared_static_methods().find(
		c_string{ u8"loadClassJVM" },
		c_string{ u8"(Ljava/lang/String;)Ljava/lang/Class;" }
	).native_function((void*)+[](
		native_environment*,
		o<jl::string>* name
	) -> o<jl::object>* {
		expected<::c&, reference> possible_c = name->view_on_stack_as_utf8(
			[](span<utf8::unit> name_utf8) -> expected<::c&, reference> {
				for(utf8::unit& cp : name_utf8) {
					if(cp == '.') cp = '/';
				}
				return
					classes.try_load_class_by_bootstrap_class_loader(name_utf8);
			}
		);
		if(possible_c.is_unexpected()) {
			thrown_in_native = possible_c.move_unexpected();
			return nullptr;
		}

		::c& c = possible_c.get_expected();

		return c.object_ptr();
	});

	c.declared_instance_methods().find(
		c_string{ u8"findLoadedClass" },
		c_string{ u8"(Ljava/lang/String;)Ljava/lang/Class;" }
	).native_function((void*)+[](
		native_environment*,
		o<jl::c_loader>* ths,
		o<jl::string>* name
	) -> o<jl::object>* {
		optional<::c&> possible_c
		= name->view_on_stack_as_utf8(
			[&](span<utf8::unit> name_utf8) -> optional<::c&> {
				for(utf8::unit& cp : name_utf8) {
					if(cp == '.') cp = '/';
				}

				return classes.try_find_class_which_loading_was_initiated_by(
					name_utf8,
					ths
				);
			}
		);
		if(possible_c.has_no_value()) {
			return nullptr;
		}

		return possible_c->object_ptr();
	});
}