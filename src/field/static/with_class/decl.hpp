#pragma once

#include "field/static/decl.hpp"
#include "field/with_class/decl.hpp"

struct static_field_with_class : field_with_class {
	using base_type = field_with_class;

	static_field_with_class(::static_field& field, ::_class& _class) :
		base_type{ (::field&) field, _class }
	{}

	const ::static_field& static_field() const {
		return (::static_field&) field();
	}

	::static_field& static_field() {
		return (::static_field&) field();
	}

};