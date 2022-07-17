#pragma once

#include "../primitives.hpp"
#include "../object/reference/decl.hpp"
#include "class/file/descriptor/type.hpp"
#include <core/meta/elements/one_of.hpp>

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
		}
		else if constexpr(same_as<DescriptorType, C>) {
			*this = jchar{};
		}
		else if constexpr(same_as<DescriptorType, D>) {
			*this = jdouble{};
		}
		else if constexpr(same_as<DescriptorType, F>) {
			*this = jfloat{};
		}
		else if constexpr(same_as<DescriptorType, I>) {
			*this = jint{};
		}
		else if constexpr(same_as<DescriptorType, J>) {
			*this = jlong{};
		}
		else if constexpr(same_as<DescriptorType, S>) {
			*this = jshort{};
		}
		else if constexpr(same_as<DescriptorType, Z>) {
			*this = jbool{};
		}
		else if constexpr(same_as<DescriptorType, object_type>) {
			*this = reference{};
		}
		else if constexpr(same_as<DescriptorType, array_type>) {
			*this = reference{};
		}
		else {
			return false;
		}
		return true;
	}
};