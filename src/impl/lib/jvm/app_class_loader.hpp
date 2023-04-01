#include "decl/classes.hpp"
#include "decl/load_class.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/define/class.hpp"

static void init_jvm_app_class_loader() {
	_class& c = load_class(c_string{"jvm/AppClassLoader"});
	c.declared_instance_methods().find(
		c_string{"findClass"},
		c_string{"(Ljava/lang/String;)Ljava/lang/Class;"}
	).native_function(
		(void*)+[](object* ths, object* name) -> object* {
			optional<posix::memory_for_range_of<unsigned char>> possible_data
				= view_string_on_stack_as_utf8(*name, [&](auto name_utf8) {
					return try_load_class_file_data_at(
						c_string{"."},
						name_utf8
					);
				}
			);
			if(possible_data.has_no_value()) {
				return nullptr;
			}

			posix::memory_for_range_of<unsigned char> data
				= move(possible_data.get());

			_class& c = [&]() -> _class& {
				auto& m = classes.mutex();
				m->lock();
				on_scope_exit unlock_classes_mutex { [&] {
					m->unlock();
				}};

				return define_class(move(data), *ths);
			}();

			return & c.instance().unsafe_release_without_destroing();
		}
	);
}