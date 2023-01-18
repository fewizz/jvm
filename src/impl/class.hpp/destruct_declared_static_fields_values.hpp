#pragma once

#include "decl/class.hpp"

void _class::destruct_declared_static_fields_values() {
	if(initialisation_state_ != initialisation_state::done) return;

	declared_static_fields_.for_each_index([&](nuint index) {
		view_ptr(index, [&]<typename Type>(Type* value) {
			value->~Type();
		});
	});
}