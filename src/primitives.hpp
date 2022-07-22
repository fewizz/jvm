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
	jint(Type value) : value { value } {}

	operator const int32& () const { return value; }
	operator int32& () { return value; }
};

struct jlong {
	int64 value;

	explicit jlong() = default;

	template<typename Type>
	requires (integer<Type> && sizeof(Type) <= sizeof(int64))
	jlong(Type value) : value { value } {}

	operator int64 () const { return value; }
};

struct jfloat {
	float value;

	explicit jfloat() = default;
	
	template<typename Type>
	requires (same_as<Type, float>)
	jfloat(Type value) : value { value } {}

	operator float () const { return value; }
};

struct jdouble {
	double value;

	explicit jdouble() = default;
	
	template<typename Type>
	requires (same_as<Type, double>)
	jdouble(Type value) : value { value } {}

	operator double () const { return value; }
};

#include <core/meta/elements/optional.hpp>

struct _class;

inline optional<_class&> void_class{};
inline optional<_class&> bool_class{};
inline optional<_class&> byte_class{};
inline optional<_class&> short_class{};
inline optional<_class&> char_class{};
inline optional<_class&> int_class{};
inline optional<_class&> long_class{};
inline optional<_class&> float_class{};
inline optional<_class&> double_class{};