#pragma once

#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"

#include <optional.hpp>

static optional<method&> wrong_method_type_exception_constructor;

inline expected<reference, reference> try_create_wrong_method_type_exception() {
	expected<reference, reference> possible_ref = try_create_object(
		wrong_method_type_exception_constructor->_class()
	);

	if(possible_ref.is_unexpected()) {
		return unexpected{ move(possible_ref.get_unexpected()) };
	}

	reference ref = move(possible_ref.get_expected());
	
	optional<reference> possible_throwable
		= try_execute(wrong_method_type_exception_constructor.get(), ref);
	
	if(possible_ref.is_unexpected()) {
		return unexpected{ move(possible_throwable.get()) };
	}

	return ref;
}