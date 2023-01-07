#pragma once

#include "./has_name_and_descriptor_equal_to.hpp"
#include <posix/abort.hpp>

template<typename Type>
struct find_by_name_and_descriptor_extension {
private:
	decltype(auto) derived() const {
		return (const Type&) *this;
	}
	decltype(auto) derived()       {
		return (const Type&) *this;
	}
public:

	template<basic_range Name, basic_range Descriptor>
	auto try_find(Name&& name, Descriptor&& descriptor) {
		return derived().try_find_last_satisfying(
			has_name_and_descriptor_equal_to{ name, descriptor }
		);
	}

	template<basic_range Name, basic_range Descriptor>
	auto try_find_index_of(Name&& name, Descriptor&& descriptor) {
		return derived().try_find_index_of_last_satisfying(
			has_name_and_descriptor_equal_to{ name, descriptor }
		);
	}

	template<basic_range Name, basic_range Descriptor>
	decltype(auto) find(Name&& name, Descriptor&& descriptor) {
		return try_find(
			forward<Name>(name), forward<Descriptor>(descriptor)
		).if_has_no_value([] { posix::abort(); }).get();
	}

	template<basic_range Name, basic_range Descriptor>
	auto find_index_of(Name&& name, Descriptor&& descriptor) {
		return try_find_index_of(
			forward<Name>(name), forward<Descriptor>(descriptor)
		).if_has_no_value([] { posix::abort(); }).get();
	}

};

template<basic_range Range>
struct find_by_name_and_descriptor_view :
	range_extensions<find_by_name_and_descriptor_view<Range>>,
	find_by_name_and_descriptor_extension<
		find_by_name_and_descriptor_view<Range>
	>
{
private:
	Range range_;
public:

	find_by_name_and_descriptor_view(Range&& range) :
		range_{ forward<Range>(range) }
	{}

	auto iterator() const { return range_iterator(range_); }
	auto iterator()       { return range_iterator(range_); }
	auto sentinel() const { return range_sentinel(range_); }
	auto sentinel()       { return range_sentinel(range_); }

};

template<basic_range Range>
find_by_name_and_descriptor_view(Range&&)
	-> find_by_name_and_descriptor_view<Range>;