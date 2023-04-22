#include "decl/classes.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/classes.hpp"
#include "decl/try_load_class_file_data_at.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"

static void init_jvm_app_class_loader() {
	_class& c = classes.load_class_by_bootstrap_class_loader(
		c_string{"jvm/AppClassLoader"}
	);

	c.declared_instance_methods().find(
		c_string{"findClass"},
		c_string{"(Ljava/lang/String;)Ljava/lang/Class;"}
	).native_function(
		(void*)+[](native_environment*, object* ths, object* name) -> object* {
			return view_string_on_stack_as_utf8(
				*name,
				[&](auto name_utf8) -> object* {
					optional<posix::memory_for_range_of<unsigned char>>
						possible_data = try_load_class_file_data_at(
							c_string{"."},
							name_utf8
						);

					if(possible_data.has_no_value()) {
						return nullptr;
					}

					posix::memory_for_range_of<unsigned char> data
						= possible_data.move();

					expected<_class&, reference> possible_c
						= classes.try_define_class(
							name_utf8, move(data), ths
						);

					if(possible_c.is_unexpected()) {
						thrown_in_native = move(
							possible_c.get_unexpected()
						);
						return nullptr;
					}

					_class& c = possible_c.get_expected();

					return & c.instance()
						.unsafe_release_without_destroing();
				}
			);
		}
	);
}