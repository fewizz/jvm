#pragma once

#pragma once

#include "class.hpp"
#include "class/es/load.hpp"
#include "object/create.hpp"

#include <core/optional.hpp>
#include <core/span.hpp>

static optional<_class&> negative_array_size_exception_class;
static optional<method&> negative_array_size_exception_constructor;

inline reference create_negative_array_size_exception() {
	_class& c = negative_array_size_exception_class.value();
	method& m = negative_array_size_exception_constructor.value();

	reference ref = create_object(c);
	stack_entry se{ ref };
	span<stack_entry, uint16> args{ &se, 1 };

	execute(method_with_class{ m, c }, args);

	return move(ref);
}

inline void init_java_lang_negative_array_size_exception() {
	negative_array_size_exception_class =
		load_class(c_string{ "java/lang/NegativeArraySizeException" });

	negative_array_size_exception_constructor =
		negative_array_size_exception_class.value()
		.find_method(c_string{ "<init>" }, c_string{ "()V" });
}