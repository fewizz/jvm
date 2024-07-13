#pragma once

#include "decl/class.hpp"

void c::reset() {
	trampolines::reset();
	instance_ = nullptr;
	defining_loader_ = nullptr;

	if(initialisation_state_ != initialisation_state::done) {
		return;
	}

	for (declared_static_field_index index : declared_static_fields().index_view()) {
		view_ptr(
			index,
			[&]<typename Type>(Type* value) {
				value->~Type();
			}
		);
	}
}