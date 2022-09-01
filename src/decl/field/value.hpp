#pragma once

#include "primitives.hpp"
#include "object/reference.hpp"

#include "abort.hpp"

#include <class_file/descriptor/method_reader.hpp>

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

		class_file::descriptor::read_type(
			descriptor.iterator(),
			[&]<typename DescriptorType>(DescriptorType) {
				using namespace class_file::descriptor;
				if constexpr(same_as<DescriptorType, b>) {
					jtype = jbyte{};
				}
				else if constexpr(same_as<DescriptorType, c>) {
					jtype = jchar{};
				}
				else if constexpr(same_as<DescriptorType, d>) {
					jtype = jdouble{};
				}
				else if constexpr(same_as<DescriptorType, f>) {
					jtype = jfloat{};
				}
				else if constexpr(same_as<DescriptorType, i>) {
					jtype = jint{};
				}
				else if constexpr(same_as<DescriptorType, j>) {
					jtype = jlong{};
				}
				else if constexpr(same_as<DescriptorType, s>) {
					jtype = jshort{};
				}
				else if constexpr(same_as<DescriptorType, z>) {
					jtype = jbool{};
				}
				else if constexpr(
					same_as<DescriptorType, class_file::descriptor::object>
				) {
					jtype = reference{};
				}
				else if constexpr(
					same_as<DescriptorType, class_file::descriptor::array>
				) {
					jtype = reference{};
				}
			},
			[](auto){ abort(); }
		);
		return jtype.value();
	}
};