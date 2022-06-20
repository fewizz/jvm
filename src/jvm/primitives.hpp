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

	template<typename Type>
	requires (integer<Type> && sizeof(Type) <= sizeof(int32))
	explicit jint(Type value) : value { value } {}

	operator int32 () const { return value; }
};

struct jlong {
	int64 value;

	explicit jlong() = default;

	template<typename Type>
	requires (integer<Type> && sizeof(Type) <= sizeof(int64))
	explicit jlong(Type value) : value { value } {}

	operator int64 () const { return value; }
};

struct jfloat {
	float value;

	explicit jfloat() = default;
	explicit jfloat(float value) : value { value } {}
};

struct jdouble {
	double value;

	explicit jdouble() = default;
	explicit jdouble(double value) : value { value } {}
};