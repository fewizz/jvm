#include "decl/class.hpp"
#include "execute.hpp"

#include <c_string.hpp>
#include <ranges.hpp>
#include <on_scope_exit.hpp>

inline optional<reference> c::try_initialise_if_need() {
	mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(
		initialisation_state_ != initialisation_state::not_started
	) {
		return {};
	}

	initialisation_state_ = initialisation_state::pending;

	if(has_super()) {
		optional<reference> possible_throwable
			= super().try_initialise_if_need();

		if(possible_throwable.has_value()) {
			return possible_throwable.move();
		}
	}

	// create declared static fields values
	for (declared_static_field_index field_index : declared_static_fields().index_view()) {
		view_ptr(field_index, []<typename Type>(Type* ptr) {
			new (ptr) Type();
		});
	}

	// 2.9.2 The requirement for ACC_STATIC was introduced in Java SE 7

	if(initialisation_method_.has_value()) {
		method& clinit = initialisation_method_.get();
		optional<reference> possible_throwable = try_execute(clinit);
		if(possible_throwable.has_value()) {
			return possible_throwable;
		}
	}

	initialisation_state_ = initialisation_state::done;

	return {};
}