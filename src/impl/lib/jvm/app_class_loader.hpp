#include "decl/classes.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/classes.hpp"
#include "decl/try_load_class_file_data_at.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"

static void init_jvm_app_class_loader() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		u8"jvm/AppClassLoader"sv
	);

	c.declared_instance_methods().find(
		u8"loadClass"sv,
		u8"(Ljava/lang/String;Z)Ljava/lang/Class;"sv
	).native_function(
	(void*)+[](
		native_environment*,
		j::c_loader* ths,
		j::string* name,
		[[maybe_unused]] bool resolve
	) -> object* {
		return name->view_on_stack_as_utf8([&](auto name_utf8) -> object* {
			for(utf8::unit& cp : name_utf8) {
				if(cp == '.') cp = '/'; // from binary name
			}

			// check if the class has already been loaded
			optional<::c&> possible_loaded_class
				= classes.try_find_class_which_loading_was_initiated_by(
					name_utf8,
					ths
				);
			if(possible_loaded_class.has_value()) {
				return possible_loaded_class->object_ptr();
			}

			// load class data at cwd
			optional<posix::memory<>> possible_data
				= try_load_class_file_data_at(
					u8"."sv, name_utf8
				);
			
			if(possible_data.has_value()) {
				posix::memory<> data = possible_data.move();

				expected<::c&, reference> possible_c
					= classes.try_define_class(
						name_utf8, move(data), ths
					);
				if(possible_c.is_unexpected()) {
					thrown_in_native = possible_c.move_unexpected();
					return nullptr;
				}
				return possible_c.get_expected().object_ptr();
			}
			// else load internally
			return classes
				.load_class_by_bootstrap_class_loader(name_utf8).object_ptr();
		});
	});

	c.declared_instance_methods().find(
		u8"findClass"sv,
		u8"(Ljava/lang/String;)Ljava/lang/Class;"sv
	).native_function(
		(void*)+[](
			native_environment*,
			j::c_loader* ths,
			j::string* name
		) -> object* {
			return name->view_on_stack_as_utf8([&](span<utf8::unit> name_utf8)
			-> object* {
				for(utf8::unit& cp : name_utf8) {
					if(cp == '.') cp = '/'; // from binary name
				}

				optional<posix::memory<>>
					possible_data = try_load_class_file_data_at(
						u8"."sv, name_utf8
					);
				if(possible_data.has_no_value()) {
					thrown_in_native
						= try_create_class_not_found_exception().get();
					return nullptr;
				}

				posix::memory<> data = possible_data.move();

				expected<::c&, reference> possible_c
					= classes.try_define_class(
						name_utf8, move(data), ths
					);

				if(possible_c.is_unexpected()) {
					thrown_in_native = possible_c.move_unexpected();
					return nullptr;
				}

				return possible_c.get_expected().object_ptr();
			});
		}
	);
}