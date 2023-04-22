#pragma once

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

using void_t = decltype(nullptr);

#include <integer.hpp>
#include <type.hpp>
#include <optional.hpp>

template<typename From, typename To>
inline constexpr bool widening_conversion_allowed =
	(
		same_as<From, int8> &&
		same_as_any<To, int16, int32, int64, float, double>
	) ||
	(
		same_as<From, int16> &&
		same_as_any<To, int32, int64, float, double>
	) ||
	(
		same_as<From, uint16> &&
		same_as_any<To, int32, int64, float, double>
	) ||
	( same_as<From, int32> && same_as_any<To, int64, float, double>) ||
	( same_as<From, int64> && same_as_any<To, float, double>) ||
	( same_as<From, float> && same_as_any<To, double>);

template<typename Type>
struct class_by_primitive_type_t {
	_class& operator () () const requires same_as<Type, bool> {
		return bool_class.get();
	}
	_class& operator () () const requires same_as<Type, int8> {
		return byte_class.get();
	}
	_class& operator () () const requires same_as<Type, int16> {
		return short_class.get();
	}
	_class& operator () () const requires same_as<Type, uint16> {
		return char_class.get();
	}
	_class& operator () () const requires same_as<Type, int32> {
		return int_class.get();
	}
	_class& operator () () const requires same_as<Type, int64> {
		return long_class.get();
	}
	_class& operator () () const requires same_as<Type, float> {
		return float_class.get();
	}
	_class& operator () () const requires same_as<Type, double> {
		return double_class.get();
	}
	_class& operator () () const requires same_as<Type, void_t> {
		return void_class.get();
	}
};

template<typename Type>
static constexpr class_by_primitive_type_t<Type> class_by_primitive_type{};

#include <overloaded.hpp>

template<typename Type>
static constexpr auto primitive_identifier = overloaded {
	[]<same_as<bool>>()   { return 'Z'; },
	[]<same_as<int8>>()   { return 'B'; },
	[]<same_as<int16>>()  { return 'S'; },
	[]<same_as<uint16>>() { return 'C'; },
	[]<same_as<int32>>()  { return 'I'; },
	[]<same_as<int64>>()  { return 'J'; },
	[]<same_as<float>>()  { return 'F'; },
	[]<same_as<double>>() { return 'D'; },
	[]<same_as<void_t>>() { return 'V'; }
}.template operator()<Type>();

#include "decl/lib/java/lang/boolean.hpp"
#include "decl/lib/java/lang/byte.hpp"
#include "decl/lib/java/lang/short.hpp"
#include "decl/lib/java/lang/character.hpp"
#include "decl/lib/java/lang/integer.hpp"
#include "decl/lib/java/lang/long.hpp"
#include "decl/lib/java/lang/float.hpp"
#include "decl/lib/java/lang/double.hpp"

template<typename Type>
struct wrapper_class_by_primitive_type_t {
	_class& operator() () const requires same_as<Type, bool> {
		return java_lang_boolean_class.get();
	}
	_class& operator() () const requires same_as<Type, int8> {
		return java_lang_byte_class.get();
	}
	_class& operator() () const requires same_as<Type, int16> {
		return java_lang_short_class.get();
	}
	_class& operator() () const requires same_as<Type, uint16> {
		return java_lang_character_class.get();
	}
	_class& operator() () const requires same_as<Type, int32> {
		return java_lang_integer_class.get();
	}
	_class& operator() () const requires same_as<Type, int64> {
		return java_lang_long_class.get();
	}
	_class& operator() () const requires same_as<Type, float> {
		return java_lang_float_class.get();
	}
	_class& operator() () const requires same_as<Type, double> {
		return java_lang_double_class.get();
	}
};

template<typename Type>
static constexpr wrapper_class_by_primitive_type_t<Type>
	wrapper_class_by_primitive_type{};

template<typename Type>
struct wrapper_value_field_position_by_primitive_type_t {
	layout::position operator() () const requires same_as<Type, bool> {
		return java_lang_boolean_value_field_position;
	}
	layout::position operator() () const requires same_as<Type, int8> {
		return java_lang_byte_value_field_position;
	}
	layout::position operator() () const requires same_as<Type, int16> {
		return java_lang_short_value_field_position;
	}
	layout::position operator() () const requires same_as<Type, uint16> {
		return java_lang_character_value_field_position;
	}
	layout::position operator() () const requires same_as<Type, int32> {
		return java_lang_integer_value_field_position;
	}
	layout::position operator() () const requires same_as<Type, int64> {
		return java_lang_long_value_field_position;
	}
	layout::position operator() () const requires same_as<Type, float> {
		return java_lang_float_value_field_position;
	}
	layout::position operator() () const requires same_as<Type, double> {
		return java_lang_double_value_field_position;
	}
};

template<typename Type>
static constexpr wrapper_value_field_position_by_primitive_type_t<Type>
	wrapper_value_field_position_by_primitive_type{};

template<typename Type>
struct wrapper_constructor_by_primitive_type_t {
	method& operator () () const requires same_as<Type, bool> {
		return java_lang_boolean_constructor.get();
	}
	method& operator () () const requires same_as<Type, int8> {
		return java_lang_byte_constructor.get();
	}
	method& operator () () const requires same_as<Type, int16> {
		return java_lang_short_constructor.get();
	}
	method& operator () () const requires same_as<Type, uint16> {
		return java_lang_character_constructor.get();
	}
	method& operator () () const requires same_as<Type, int32> {
		return java_lang_integer_constructor.get();
	}
	method& operator () () const requires same_as<Type, int64> {
		return java_lang_long_constructor.get();
	}
	method& operator () () const requires same_as<Type, float> {
		return java_lang_float_constructor.get();
	}
	method& operator () () const requires same_as<Type, double> {
		return java_lang_double_constructor.get();
	}
};

template<typename Type>
static constexpr wrapper_constructor_by_primitive_type_t<Type>
	wrapper_constructor_by_primitive_type{};