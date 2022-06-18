#pragma once

#include <core/integer.hpp>

struct jvoid{};

struct jbool {
	bool value;
};

struct jbyte {
	uint8 value;
};

struct jshort {
	int16 value;
};

struct jchar {
	int16 value;
};

struct jint {
	int32 value;

	explicit jint() = default;
	explicit jint(int32 value) : value { value } {}

	operator int32 () const { return value; }
};

struct jlong {
	int64 value;

	explicit jlong() = default;
	explicit jlong(int64 value) : value { value } {}

	operator int64 () const { return value; }
};

struct jfloat {
	float value;
};

struct jdouble {
	double value;
};