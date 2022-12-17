#pragma once

#include "class/layout.hpp"
#include "class/layout_view_extension.hpp"
#include "class/member_index.hpp"

#include <optional.hpp>
#include <integer.hpp>

#include <posix/memory.hpp>

struct _class;
struct reference;

struct object : layout_view_extension<object> {
private:
	uint32 references_ = 0;
	optional<_class&> class_;
	posix::memory_for_range_of<uint8> data_;

	friend reference;

	friend reference create_object(_class& c);
	void on_reference_added();
	void on_reference_removed();
	void unsafe_decrease_reference_count_without_destroing();

	// required member functions for layout_view_extension:
	friend layout_view_extension<object>;

	inline const ::layout& layout_for_view();
	uint8* data_for_layout_view() { return data_.as_span().begin(); }
	inline auto fields_view_for_layout_view();

public:

	object(_class& c);
	~object();

	const ::_class& _class() const { return class_.get(); }
	      ::_class& _class()       { return class_.get(); }

	uint32 references() { return references_; }

};

inline reference create_object(_class& c);