#include "decl/lib/java/lang/negative_array_size_exception.hpp"

#include "decl/execute.hpp"
#include "decl/classes.hpp"
#include "decl/object.hpp"

[[nodiscard]] inline expected<reference, reference>
try_create_negative_array_size_exception() {
	_class& c = negative_array_size_exception_class.get();
	method& m = negative_array_size_exception_constructor.get();

	expected<reference, reference> possible_ref = try_create_object(c);
	if(possible_ref.is_unexpected()) {
		return unexpected{ move(possible_ref.get_unexpected()) };
	}
	reference ref = move(possible_ref.get_expected());
	optional<reference> possible_throwable = try_execute(m, ref);

	if(possible_throwable.has_value()) {
		return unexpected{ move(possible_throwable.get()) };
	}
	return ref;
}

inline void init_java_lang_negative_array_size_exception() {
	negative_array_size_exception_class
		= classes.load_class_by_bootstrap_class_loader(
			c_string{ "java/lang/NegativeArraySizeException" }
		);

	negative_array_size_exception_constructor =
		negative_array_size_exception_class
		.get().instance_methods().find(
			c_string{ "<init>" }, c_string{ "()V" }
		);
}