#pragma once

#include "declared_instance_field_index.hpp"
#include "../method/with_class/decl.hpp"
#include "../field/static/with_class/decl.hpp"
#include "../method/decl.hpp"
#include "../object/reference/decl.hpp"
#include "../../alloc.hpp"
#include <core/meta/elements/one_of.hpp>
#include <core/limited_list.hpp>

struct _class;
struct static_field;

using trampoline_entry = elements::one_of<
	elements::none, _class&, method_with_class,
	instance_field_index, static_field_with_class,
	reference
>;

struct trampoline_pool :
	protected limited_list<trampoline_entry, uint16, default_allocator>
{
	using base_type = limited_list<trampoline_entry, uint16, default_allocator>;

	trampoline_pool(uint16 size) : base_type{ size } {
		while(size > 0) {
			emplace_back(elements::none{});
			--size;
		}
	}

	using base_type::emplace_back;

	uint16 trampolines_count() const {
		return capacity();
	}

	const auto& trampoline(uint16 index) const { return (*this)[index - 1]; }
	auto& trampoline(uint16 index) { return (*this)[index - 1]; }

};