#include "decl/lib/java/lang/array_store_exception.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"

#include <optional.hpp>

static optional<_class&> array_store_exception_class;
static optional<method&> array_store_exception_constructor;

static void init_java_lang_array_store_exception() {
	array_store_exception_class
		= classes.find_or_load(c_string{ "java/lang/ArrayStoreException" });

	array_store_exception_constructor =
		array_store_exception_class->declared_methods().find(
			c_string{"<init>"}, c_string{ "()V" }
		);
}

static inline reference create_array_store_exception() {
	reference o = create_object(array_store_exception_class.get());
	stack.emplace_back(o);
	execute(array_store_exception_constructor.get());
	return move(o);
}