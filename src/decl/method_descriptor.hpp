#pragma once

#include <range.hpp>
#include <list.hpp>
#include <exchange.hpp>
#include <class_file/descriptor/method_reader.hpp>

#include <posix/memory.hpp>

using one_of_descriptor_types = variant<
	class_file::z, class_file::b,
	class_file::c, class_file::s,
	class_file::i, class_file::f,
	class_file::j, class_file::d,
	class_file::v,
	class_file::array, class_file::object
>;

struct _class;

struct parameter_type : one_of_descriptor_types {
	using one_of_descriptor_types::one_of_descriptor_types;
};

class return_type : public one_of_descriptor_types {
	using base_type = one_of_descriptor_types;
public:
	using base_type::base_type;

	return_type() : base_type{ class_file::v{} } {}
};

template<sized_range Range>
class method_descriptor : public range_extensions<method_descriptor<Range>> {
	Range                                            range_;
	list<posix::memory_for_range_of<parameter_type>> parameters_types_;
	return_type                                      return_type_;

public:

	method_descriptor(Range&& range) : range_{ forward<Range>(range) } {
		class_file::method_descriptor::reader reader {
			range_iterator(range_)
		};

		parameters_types_ = {
			posix::allocate_memory_for<parameter_type>(
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
	class_file::method_descriptor::reader reader {
		range_iterator(descriptor)
	};
	return reader.try_read_parameters_count([](auto){ abort(); }).value();
}