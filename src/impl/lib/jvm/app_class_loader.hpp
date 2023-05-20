#include "decl/classes.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/classes.hpp"
#include "decl/try_load_class_file_data_at.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"

static void init_jvm_app_class_loader() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"jvm/AppClassLoader" }
	);

	c.declared_instance_methods().find(
		c_string{ u8"findClass" },
		c_string{ u8"(Ljava/lang/String;)Ljava/lang/Class;" }
	).native_function(
		(void*)+[](
			native_environment*,
			j::c_loader* ths,
			j::string* name
		) -> object* {
			return name->view_on_stack_as_utf8(
				[&](auto name_utf8) -> object* {
					optional<posix::memory<>>
						possible_data = try_load_class_file_data_at(
							c_string{ u8"." },
							name_utf8
						);

					if(possible_data.has_no_value()) {
						return nullptr;
					}

					posix::memory<> data
						= possible_data.move();

					expected<::c&, reference> possible_c
						= classes.try_define_class(
							name_utf8, move(data), ths
						);

					if(possible_c.is_unexpected()) {
						thrown_in_native = move(
							possible_c.get_unexpected()
						);
						return nullptr;
					}

					::c& c = possible_c.get_expected();
					return c.object_ptr();
				}
			);
		}
	);
}