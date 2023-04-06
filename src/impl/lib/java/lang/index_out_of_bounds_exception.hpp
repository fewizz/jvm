#include "decl/lib/java/lang/index_out_of_bounds_exception.hpp"

#include "decl/execute.hpp"
#include "decl/object.hpp"
#include "decl/classes.hpp"

[[nodiscard]] inline expected<reference, reference>
try_create_index_of_of_bounds_exception() {
	_class& c = index_of_of_bounds_exception_class.get();
	method& m = index_of_of_bounds_exception_constructor.get();
	expected<reference, reference> possible_ref = try_create_object(c);
	if(possible_ref.is_unexpected()) {
		return unexpected{ move(possible_ref.get_unexpected()) };
	}
	reference ref = move(possible_ref.get_expected());
	optional<reference> possible_throwable = try_execute(m, ref);
	if(possible_throwable.has_value()) {
		return unexpected{ possible_throwable.get() };
	}
	return ref;
}

inline void init_java_lang_index_of_of_bounds_exception() {
	index_of_of_bounds_exception_class
		= classes.load_class_by_bootstrap_class_loader(
			c_string{ "java/lang/IndexOutOfBoundsException" }
		);

	index_of_of_bounds_exception_constructor =
		index_of_of_bounds_exception_class.get()
		.instance_methods().find(
			c_string{ "<init>" }, c_string{ "()V" }
		);
}