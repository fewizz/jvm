#pragma once

#include "./member_index.hpp"
#include <posix/abort.hpp>
#include <print/print.hpp>

template<typename Type, typename IndexType>
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
		return derived().try_find_first_satisfying([&](auto& member) {
			return member.has_name_and_descriptor_equal_to(name, descriptor);
		});
	}

	template<basic_range Name, basic_range Descriptor>
	optional<IndexType> try_find_index_of(
		Name&& name, Descriptor&& descriptor
	) {
		auto possible_index = derived().try_find_index_of_first_satisfying(
			[&](auto& mem) {
				return mem.has_name_and_descriptor_equal_to(name, descriptor);
			}
		);
		if(!possible_index.has_value()) {
			return {};
		}
		return { possible_index.get() };
	}

	template<basic_range Name, basic_range Descriptor>
	decltype(auto) find(Name&& name, Descriptor&& descriptor) {
		return try_find(
			forward<Name>(name), forward<Descriptor>(descriptor)
		).if_has_no_value([] { posix::abort(); }).get();
	}

	template<basic_range Name, basic_range Descriptor>
	IndexType find_index_of(Name&& name, Descriptor&& descriptor) {
		return try_find_index_of(
			forward<Name>(name), forward<Descriptor>(descriptor)
		).if_has_no_value([&] {
			print::err("couldn't find class member with name \"", name, "\"\n");
			posix::abort();
		}).get();
	}

	optional<IndexType> try_find_index_of(auto& value) {
		return derived().try_find_index_of_first_satisfying([&](auto& value0) {
			return &value0 == &value;
		});
	}

	IndexType find_index_of(auto& value) {
		optional<IndexType> possible = try_find_index_of(value);
		if(possible.has_no_value()) {
			print::err("couldn't find class member\n");
			posix::abort();
		}
		return possible.get();
	}

};

template<basic_range Range, typename IndexType = class_member_index>
struct find_by_name_and_descriptor_view :
	range_extensions<find_by_name_and_descriptor_view<Range, IndexType>>,
	find_by_name_and_descriptor_extension<
		find_by_name_and_descriptor_view<Range, IndexType>,
		IndexType
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

	// TODO overwrite other way
	decltype(auto) operator [] (IndexType index) const {
		return range_[uint16{ index }];
	}
	decltype(auto) operator [] (IndexType index)       {
		return range_[uint16{ index }];
	}

	IndexType size() const { return IndexType(::range_size(range_)); }

	template<typename Predicate>
	optional<IndexType>
	try_find_index_of_first_satisfying(Predicate&& predicate) const {
		auto possible_index = range_.try_find_index_of_first_satisfying(
			forward<Predicate>(predicate)
		);
		if(!possible_index.has_value()) {
			return {};
		}
		return {(uint16)possible_index.get()};
	}

};