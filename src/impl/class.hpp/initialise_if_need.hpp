#include "class.hpp"

inline void _class::initialise_if_need() {
	if(
		initialisation_state_ == initialised ||
		initialisation_state_ == pending
	) {
		return;
	}

	initialisation_state_ = pending;

	auto clinit = try_find_method(c_string{ "<clinit>" });
	if(clinit.has_value()) {
		execute(
			method_with_class{ clinit.value(), *this },
			arguments_span{}
		);
	}
	initialisation_state_ = initialised;
}