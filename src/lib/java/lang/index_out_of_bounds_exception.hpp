#pragma once

#include "class/decl.hpp"
#include "classes/load.hpp"
#include "object/create.hpp"

#include <core/optional.hpp>
#include <core/span.hpp>

static optional<_class&> index_of_of_bounds_exception_class;
static optional<method&> index_of_of_bounds_exception_constructor;

inline reference create_index_of_of_bounds_exception() {
	_class& c = index_of_of_bounds_exception_class.value();
	method& m = index_of_of_bounds_exception_constructor.value();

	reference ref = create_object(c);
	stack_entry se{ ref };
	span<stack_entry, uint16> args{ &se, 1 };

	invoke(method_with_class{ m, c }, args);

	return move(ref);
}

inline void init_java_lang_index_of_of_bounds_exception() {
	index_of_of_bounds_exception_class =
		load_class(c_string{ "java/lang/IndexOutOfBoundsException" });

	index_of_of_bounds_exception_constructor =
		index_of_of_bounds_exception_class.value()
		.find_method(c_string{ "<init>" }, c_string{ "()V" });
}