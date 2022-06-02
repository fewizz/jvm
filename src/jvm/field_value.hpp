#pragma once

#include "reference.hpp"
#include <core/meta/elements/one_of.hpp>

struct jvoid{};

struct jbool {
	bool value;
};

struct jbyte {
	bool value;
};

struct jshort {
	int16 value;
};

struct jchar {
	int16 value;
};

struct jint {
	int32 value;
};

struct jlong {
	int64 value;
};

struct jfloat {
	float value;
};

struct jdouble {
	double value;
};

struct field_value : elements::one_of<
	jvoid, jbool, jbyte, jchar, jshort, jint,
	jlong, jfloat, jdouble, reference
> {
	using base_type = elements::one_of<
		jvoid, jbool, jbyte, jchar, jshort, jint,
		jlong, jfloat, jdouble, reference
	>;
	using base_type::base_type;
	using base_type::operator = ;

	field_value() : base_type{ jvoid{} } {};
};