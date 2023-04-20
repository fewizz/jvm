#pragma once

#include "class.hpp"
#include "class/layout.hpp"
#include "class/layout_view_extension.hpp"
#include "class/member_index.hpp"

#include <optional.hpp>
#include <integer.hpp>

#include <posix/memory.hpp>
#include <posix/thread.hpp>

struct _class;
struct reference;

struct object : layout_view_extension<object, instance_field_index> {
private:
	uint32 references_ = 0;
	optional<_class&> class_{};
	body<posix::mutex> mutex_{};
	posix::memory_for_range_of<uint8> data_;

	friend reference;

	friend expected<reference, reference> try_create_object(_class& c);
	void on_reference_added();
	void on_reference_removed();
	void unsafe_decrease_reference_count_without_destroing();

	// required member functions for layout_view_extension:
	friend layout_view_extension<object, instance_field_index>;

	inline const ::layout& layout_for_view() {
		return class_->instance_layout();
	}
	uint8* data_for_layout_view() { return data_.as_span().begin(); }
	inline auto fields_view_for_layout_view() {
		return class_->instance_fields();
	}

public:

	object(_class& c);
	~object();

	const ::_class& _class() const {
		if(!class_.has_value()) {
			print::err("object::_class(): object is null");
			posix::abort();
		}
		return class_.get();
	}
	      ::_class& _class()       {
		if(!class_.has_value()) {
			print::err("object::_class(): object is null");
			posix::abort();
		}
		return class_.get();
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
try_create_object(_class& c);

#include "execute.hpp"

template<typename... Args>
[[nodiscard]] inline expected<reference, reference>
try_create_object(method& constructor, Args&&... args) {
	expected<reference, reference> possible_ref
		= try_create_object(constructor._class());
	
	if(possible_ref.is_unexpected()) {
		return unexpected{ move(possible_ref.get_unexpected()) };
	}

	reference ref = move(possible_ref.get_expected());

	stack.emplace_back(ref);

	(stack.emplace_back(forward<Args>(args)), ...);

	optional<reference> possible_throwable = try_invoke_special_selected(
		constructor
	);

	if(possible_throwable.has_value()) {
		return unexpected{ move(possible_throwable.get()) };
	}

	return ref;
}