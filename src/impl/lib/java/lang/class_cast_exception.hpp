#include "decl/lib/java/lang/class_cast_exception.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"

#include <optional.hpp>

static optional<_class&> class_cast_exception_class;
static optional<method&> class_cast_exception_constructor;

static void init_java_lang_class_cast_exception() {
	class_cast_exception_class
		= classes.find_or_load(c_string{ "java/lang/ClassCastException" });

	class_cast_exception_constructor =
		class_cast_exception_class->declared_methods().find(
			c_string{"<init>"}, c_string{ "()V" }
		);
}

static inline reference create_class_cast_exception() {
	reference o = create_object(class_cast_exception_class.value());
	stack.emplace_back(o);
	execute(class_cast_exception_constructor.value());
	return move(o);
}