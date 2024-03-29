#pragma once

#include "decl/class.hpp"
#include "decl/class/layout_view_extension.hpp"
#include "decl/class/member_index.hpp"

#include <optional.hpp>
#include <integer.hpp>

#include <posix/memory.hpp>
#include <posix/thread.hpp>

struct layout;

struct _class;
struct reference;

struct object :
	layout_view_extension<object, instance_field_index>
{
private:
	uint32 references_ = 0;
	c& class_;
	body<posix::mutex> mutex_{};
	posix::memory<> data_;

	friend reference;

	friend expected<reference, reference> try_create_object(c& c);

	void on_reference_added();
	void on_reference_removed();
	void unsafe_decrease_reference_count_without_destroing();

	// required member functions for layout_view_extension:
	friend layout_view_extension<object, instance_field_index>;

	const ::layout& layout_for_view() {
		return class_.instance_layout();
	}
	uint8* data_for_layout_view() { return data_.begin(); }

	auto fields_view_for_layout_view() {
		return class_.instance_fields();
	}

public:

	object(const object&  c) = delete;
	object(      object&& c) = delete;

	object(c& c);
	~object();

	const ::c& c() const {
		//if(!class_.has_value()) {
		//	print::err("object::c(): object is null");
		//	posix::abort();
		//}
		return class_;
	}
		  ::c& c()       {
		//if(!class_.has_value()) {
		//	print::err("object::c(): object is null");
		//	posix::abort();
		//}
		return class_;
	}

	uint32 references() { return references_; }

	void lock() {
		mutex_->lock();
	}

	void unlock() {
		mutex_->unlock();
	}

};

[[nodiscard]] inline expected<reference, reference>
try_create_object(c& c);

#include "execute.hpp"

template<typename... Args>
[[nodiscard]] expected<reference, reference>
try_create_object(instance_method& constructor, Args&&... args) {
	expected<reference, reference> possible_ref
		= try_create_object(constructor.c());

	if(possible_ref.is_unexpected()) {
		return unexpected{ possible_ref.move_unexpected() };
	}

	reference ref = possible_ref.move_expected();

	stack.emplace_back(ref);

	(stack.emplace_back(forward<Args>(args)), ...);

	optional<reference> possible_throwable = try_invoke_special_selected(
		constructor
	);

	if(possible_throwable.has_value()) {
		return unexpected{ possible_throwable.move() };
	}

	return ref;
}