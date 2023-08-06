#pragma once

#include "decl/class.hpp"

void c::reset() {
	trampolines::reset();
	instance_ = nullptr;
	defining_loader_ = nullptr;

	if(initialisation_state_ != initialisation_state::done) {
		return;
	}

	declared_static_fields().for_each_index(
		[&](declared_static_field_index index) {
			view_ptr(
				declared_static_field_index{ index },
				[&]<typename Type>(Type* value) {
					value->~Type();
				}
			);
		}
	);
}