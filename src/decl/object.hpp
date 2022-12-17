#pragma once

#include "class/layout.hpp"
#include "class/member_index.hpp"

#include <optional.hpp>
#include <integer.hpp>

#include <posix/memory.hpp>

struct _class;
struct reference;

struct object {
private:
	uint32 references_ = 0;
	optional<_class&> class_;
	posix::memory_for_range_of<uint8> data_;

	friend reference;

	friend reference create_object(_class& c);

	void on_reference_added();

	void on_reference_removed();

	void unsafe_decrease_reference_count_without_destroing();

public:

	object(_class& c);
	~object();

	const ::_class& _class() const { return class_.get(); }
	      ::_class& _class()       { return class_.get(); }

	uint32 references() { return references_; }

	decltype(auto) view_ptr(instance_field_index index, auto&& handler);

	decltype(auto) view(instance_field_index position, auto&& handler);

	template<typename Type>
	decltype(auto) view(instance_field_position position, auto&& handler);

	template<typename Type>
	Type& get(instance_field_position position) {
		return view<Type>(position, [](Type& e) -> Type& { return e; });
	}

	template<typename Type>
	void set(instance_field_position position, Type value) {
		view<Type>(position, [&](Type& e) { return e = move(value); });
	}

};

inline reference create_object(_class& c);