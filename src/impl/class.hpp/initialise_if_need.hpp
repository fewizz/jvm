#include "decl/class.hpp"
#include "execute.hpp"

#include <c_string.hpp>
#include <ranges.hpp>

inline optional<reference> _class::try_initialise_if_need() {
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
	declared_static_fields().for_each_index(
		[&](declared_static_field_index field_index) {
			view_ptr(field_index, []<typename Type>(Type* ptr) {
				new (ptr) Type();
			});
		}
	);

	// 2.9.2 "The requirement for ACC_STATIC was introduced in Java SE 7"
	optional<method&> possible_method =
		find_by_name_and_descriptor_view {
			ranges{ declared_instance_methods(), declared_static_methods() }
			.concat_view()
		}
		.try_find(c_string{ "<clinit>" }, c_string{ "()V" });
	
	if(possible_method.has_value()) {
		method& clinit = possible_method.get();
		return try_execute(clinit);
	}

	initialisation_state_ = initialisation_state::done;

	return {};
}