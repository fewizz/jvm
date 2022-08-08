#pragma once

#include <integer.hpp>
#include <type/is_same_as.hpp>

struct jvoid{};

class jbool {
	bool value_;
public:

	jbool() = default;

	template<same_as<bool> Type>
	jbool(Type value) : value_ { value } {}

	operator const bool& () const { return value_; }
	operator bool& () { return value_; }

};

class jbyte {
	int8 value_;
public:

	jbyte() = default;

	template<typename Type>
	requires (integer<Type> && sizeof(Type) <= sizeof(int8))
	jbyte(Type value) : value_ { value } {}

	operator const int8& () const { return value_; }
	operator int8& () { return value_; }

};

class jshort {
	int16 value_;
public:

	jshort() = default;

	template<typename Type>
	requires (integer<Type> && sizeof(Type) <= sizeof(int16))
	jshort(Type value) : value_ { value } {}

	operator const int16& () const { return value_; }
	operator int16& () { return value_; }

};

class jchar {
	uint16 value_;
public:

	jchar() = default;

	template<typename Type>
	requires (integer<Type> && sizeof(Type) <= sizeof(int16))
	jchar(Type value) : value_ { value } {}

	operator const uint16& () const { return value_; }
	operator uint16& () { return value_; }

};

class jint {
	int32 value_;
public:

	jint() = default;

	template<typename Type>
	requires (integer<Type> && sizeof(Type) <= sizeof(int32))
	jint(Type value) : value_ { value } {}

	operator const int32& () const { return value_; }
	operator int32& () { return value_; }

};

class jlong {
	int64 value_;
public:

	jlong() = default;

	template<typename Type>
	requires (integer<Type> && sizeof(Type) <= sizeof(int64))
	jlong(Type value) : value_ { value } {}

	operator const int64& () const { return value_; }
	operator int64& () { return value_; }

};

class jfloat {
	float value_;
public:

	jfloat() = default;
	
	template<typename Type>
	requires (same_as<Type, float>)
	jfloat(Type value) : value_ { value } {}

	operator const float& () const { return value_; }
	operator float& () { return value_; }

};

class jdouble {
	double value_;
public:

	jdouble() = default;
	
	template<typename Type>
	requires (same_as<Type, double>)
	jdouble(Type value) : value_ { value } {}

	operator const double& () const { return value_; }
	operator double& () { return value_; }

};

#include <optional.hpp>

struct _class;

static optional<_class&> void_class{};
static optional<_class&> bool_class{};
static optional<_class&> byte_class{};
static optional<_class&> short_class{};
static optional<_class&> char_class{};
static optional<_class&> int_class{};
static optional<_class&> long_class{};
static optional<_class&> float_class{};
static optional<_class&> double_class{};