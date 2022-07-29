#pragma once

#include "class.hpp"
#include "class/es/load.hpp"
#include "object/create.hpp"

#include <core/optional.hpp>
#include <core/span.hpp>

static optional<_class&> null_pointer_exception_class;
static optional<method&> null_pointer_exception_constructor;

inline reference create_null_pointer_exception() {
	_class& c = null_pointer_exception_class.value();
	method& m = null_pointer_exception_constructor.value();

	reference ref = create_object(c);
	stack_entry se{ ref };
	span<stack_entry, uint16> args{ &se, 1 };

	execute(method_with_class{ m, c }, args);

	return move(ref);
}

inline void init_java_lang_null_pointer_exception() {
	null_pointer_exception_class =
		load_class(c_string{ "java/lang/NullPointerException" });

	null_pointer_exception_constructor =
		null_pointer_exception_class.value()
		.find_method(c_string{ "<init>" }, c_string{ "()V" });
}