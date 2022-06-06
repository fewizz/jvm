#pragma once

#include "reference.hpp"
#include "class/file/descriptor/type.hpp"
#include <core/meta/elements/one_of.hpp>

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

	void set_default_value() {
		view([&]<typename ValueType>(ValueType& value) {
			value = ValueType{};
		});
	}

	template<typename DescriptorType>
	bool set_default_value() {
		using namespace class_file::descriptor;
		if constexpr(same_as<DescriptorType, B>) {
			*this = jbyte{};
		} else
		if constexpr(same_as<DescriptorType, C>) {
			*this = jchar{};
		} else
		if constexpr(same_as<DescriptorType, D>) {
			*this = jdouble{};
		} else
		if constexpr(same_as<DescriptorType, F>) {
			*this = jfloat{};
		} else
		if constexpr(same_as<DescriptorType, I>) {
			*this = jint{};
		} else
		if constexpr(same_as<DescriptorType, J>) {
			*this = jlong{};
		} else
		if constexpr(same_as<DescriptorType, S>) {
			*this = jshort{};
		} else
		if constexpr(same_as<DescriptorType, Z>) {
			*this = jbool{};
		} else
		if constexpr(same_as<DescriptorType, object_type>) {
			*this = reference{};
		} else
		if constexpr(is_array_type<DescriptorType>) {
			if constexpr(array_type_rank<DescriptorType> < 4) {
				*this = reference{};
			} else {
				return false;
			}
		} else {
			return false;
		}
			return true;
	}
};