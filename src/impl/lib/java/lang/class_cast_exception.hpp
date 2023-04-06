#include "decl/lib/java/lang/class_cast_exception.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"

#include <optional.hpp>

static optional<_class&> class_cast_exception_class;
static optional<method&> class_cast_exception_constructor;

static void init_java_lang_class_cast_exception() {
	class_cast_exception_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/ClassCastException" }
	);

	class_cast_exception_constructor =
		class_cast_exception_class->declared_methods().find(
			c_string{"<init>"}, c_string{ "()V" }
		);
}

[[nodiscard]] inline expected<reference, reference>
try_create_class_cast_exception() {
	expected<reference, reference> possible_ref
		= try_create_object(class_cast_exception_class.get());
	if(possible_ref.is_unexpected()) {
		return unexpected{ move(possible_ref.get_unexpected()) };
	}
	reference ref = move(possible_ref.get_expected());
	stack.emplace_back(ref);
	optional<reference> possible_throwable
		= try_execute(class_cast_exception_constructor.get());
	if(possible_throwable.has_value()) {
		return unexpected{ move(possible_throwable.get()) };
	}
	return ref;
}