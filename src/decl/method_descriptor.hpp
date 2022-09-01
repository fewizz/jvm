#pragma once

#include "alloc.hpp"

#include <range.hpp>
#include <memory_list.hpp>

#include <class_file/descriptor/method_reader.hpp>

using one_of_descriptor_types = elements::one_of<
	class_file::descriptor::z, class_file::descriptor::b,
	class_file::descriptor::c, class_file::descriptor::s,
	class_file::descriptor::i, class_file::descriptor::f,
	class_file::descriptor::j, class_file::descriptor::d,
	class_file::descriptor::v,
	class_file::descriptor::array, class_file::descriptor::object
>;

struct _class;

struct parameter_type : one_of_descriptor_types {
	using one_of_descriptor_types::one_of_descriptor_types;
};

class return_type : public one_of_descriptor_types {
	using base_type = one_of_descriptor_types;
public:
	using base_type::base_type;

	return_type() : base_type{ class_file::descriptor::v{} } {}
};

template<sized_range Range>
class method_descriptor {
	Range                              range_;
	memory_list<parameter_type, uint8> parameters_types_;
	return_type                        return_type_;

public:
	~method_descriptor() {
		parameters_types_.clear();
		deallocate(parameters_types_.memory_span());
	}

	method_descriptor(Range&& range) : range_{ forward<Range>(range) } {
		class_file::descriptor::method::reader reader {
			range_iterator(range_)
		};

		parameters_types_ = {
			allocate_for<parameter_type>(
				method_descriptor_parameters_count(range_)
			)
		};

		auto return_type_reader
		= reader.try_read_parameter_types_and_get_return_type_reader(
			[&](auto parameter_type) {
				parameters_types_.emplace_back(parameter_type);
			},
			[](auto) { abort(); }
		).value();
		return_type_reader.try_read_and_get_advanced_iterator(
			[&](auto ret_type) {
				return_type_ = ret_type;
			},
			[](auto) { abort(); }
		);
	}

	method_descriptor(method_descriptor&& that) = default;

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
	class_file::descriptor::method::reader reader {
		range_iterator(descriptor)
	};
	return reader.try_read_parameters_count([](auto){ abort(); }).value();
}