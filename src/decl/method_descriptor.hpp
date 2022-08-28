#pragma once

#include "alloc.hpp"

#include <range.hpp>
#include <memory_list.hpp>

#include <class_file/descriptor/reader.hpp>

using one_of_descriptor_types = elements::one_of<
	class_file::descriptor::Z, class_file::descriptor::B,
	class_file::descriptor::C, class_file::descriptor::S,
	class_file::descriptor::I, class_file::descriptor::F,
	class_file::descriptor::J, class_file::descriptor::D,
	class_file::descriptor::V,
	class_file::descriptor::array_type, class_file::descriptor::object_type
>;

struct _class;

struct parameter_type : one_of_descriptor_types {
	using one_of_descriptor_types::one_of_descriptor_types;
};

class return_type : public one_of_descriptor_types {
	using base_type = one_of_descriptor_types;
public:
	using base_type::base_type;

	return_type() : base_type{ class_file::descriptor::V{} } {}
};

template<sized_range Range>
class method_descriptor {
	Range                              range_;
	memory_list<parameter_type, uint8> parameters_types_;
	return_type                        return_type_;

public:
	method_descriptor(Range&& range) : range_{ forward<Range>(range) } {
		parameters_types_ = {
			allocate_for<parameter_type>(
				method_descriptor_parameters_count(range_)
			)
		};

		class_file::descriptor::method_reader mr {
			range.iterator()
		};
		auto [return_type_reader, res] = mr([&](auto parameter_type) {
			parameters_types_.emplace_back(parameter_type);
			return true;
		});
		return_type_reader([&](auto ret_type) {
			return_type_ = ret_type;
			return true;
		});
	}

	auto iterator() const { return range_iterator(range_); }
	auto iterator()       { return range_iterator(range_); }

	auto sentinel() const { return range_sentinel(range_); }
	auto sentinel()       { return range_sentinel(range_); }

	auto size() const { return range_size(range_); }

	auto elements_ptr() const { return range_.elements_ptr(); }

	auto& parameters_types() const { return parameters_types_; }
	auto& return_type()      const { return return_type_; }
};

template<basic_range Range>
method_descriptor(Range&&) -> method_descriptor<Range>;

template<basic_range Descriptor>
static uint8 method_descriptor_parameters_count(Descriptor&& descriptor) {
	class_file::descriptor::method_reader mr { descriptor.iterator() };
	uint8 count = 0;
	mr([&](auto) { ++count; return true; });
	return count;
}