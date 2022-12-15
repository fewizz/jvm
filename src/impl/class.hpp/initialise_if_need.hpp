#include "decl/class.hpp"
#include "execute.hpp"

#include <c_string.hpp>

inline void _class::initialise_if_need() {
	if(
		initialisation_state_ != initialisation_state::not_started
	) {
		return;
	}

	initialisation_state_ = initialisation_state::pending;

	if(has_super()) {
		super().initialise_if_need();
	}

	declared_static_fields().for_each_indexed([&](field& f, int16 index) {
		declared_static_fields_values_[index].construct(f.descriptor());
	});

	// 2.9.2 "The requirement for ACC_STATIC was introduced in Java SE 7"
	declared_methods()
		.try_find(c_string{ "<clinit>" }, c_string{ "()V" })
		.if_has_value([](method& clinit) {
			execute(clinit);
		});

	initialisation_state_ = initialisation_state::done;
}