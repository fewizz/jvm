#pragma once

#include "primitives.hpp"
#include "object/reference.hpp"

#include "abort.hpp"

#include <class_file/descriptor/reader.hpp>

#include <elements/one_of.hpp>
#include <range.hpp>

#include <stdio.h>

using one_of_jtpyes = elements::one_of<
	reference, jbool, jbyte, jchar, jshort, jint,
	jlong, jfloat, jdouble
>;

struct field_value : one_of_jtpyes {
	using base_type = one_of_jtpyes;
	using base_type::base_type;
	using base_type::operator = ;

	template<basic_range Descriptor>
	field_value(Descriptor&& descriptor) :
		base_type{ jtype_from_descriptor(descriptor) }
	{};

private:
	template<basic_range Descriptor>
	static one_of_jtpyes jtype_from_descriptor(Descriptor&& descriptor) {

		optional<one_of_jtpyes> jtype;

		bool result = class_file::descriptor::read_field(
			descriptor.iterator(),
			[&]<typename DescriptorType>(DescriptorType) {
				using namespace class_file::descriptor;
				if constexpr(same_as<DescriptorType, B>) {
					jtype = jbyte{};
				}
				else if constexpr(same_as<DescriptorType, C>) {
					jtype = jchar{};
				}
				else if constexpr(same_as<DescriptorType, D>) {
					jtype = jdouble{};
				}
				else if constexpr(same_as<DescriptorType, F>) {
					jtype = jfloat{};
				}
				else if constexpr(same_as<DescriptorType, I>) {
					jtype = jint{};
				}
				else if constexpr(same_as<DescriptorType, J>) {
					jtype = jlong{};
				}
				else if constexpr(same_as<DescriptorType, S>) {
					jtype = jshort{};
				}
				else if constexpr(same_as<DescriptorType, Z>) {
					jtype = jbool{};
				}
				else if constexpr(same_as<DescriptorType, object_type>) {
					jtype = reference{};
				}
				else if constexpr(same_as<DescriptorType, array_type>) {
					jtype = reference{};
				}
				else {
					return false;
				}
				return true;
			}
		);
		if(!result) {
			fputs("couldn't read field descriptor", stderr); abort();
		}
		return jtype.value();
	}
};