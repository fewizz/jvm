#include "class.hpp"
#include "field.hpp"
#include "decl/class/resolve_field.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"

#include <class_file/constant.hpp>

#include <tuple.hpp>
#include <optional.hpp>

inline expected<instance_field_index_and_stack_size, reference>
_class::try_get_resolved_instance_field_index(
	class_file::constant::field_ref_index ref_index
) {
	mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(auto& t = trampoline(ref_index); t.has_value()) {
		if(!t.is_same_as<instance_field_index_and_stack_size>()) {
			posix::abort();
		}
		return t.get_same_as<instance_field_index_and_stack_size>();
	}

	expected<field&, reference> possible_resolved_field
		= try_resolve_field(*this, ref_index);
	
	if(possible_resolved_field.is_unexpected()) {
		return unexpected{ move(possible_resolved_field.get_unexpected()) };
	}

	field& resolved_field = possible_resolved_field.get_expected();
	
	if(resolved_field.is_static()) {
		expected<reference, reference> possible_icce
			= try_create_incompatible_class_change_error();
		return unexpected { move(possible_icce.get()) };
	}

	optional<instance_field_index> possible_index
		= resolved_field._class().instance_fields()
		.try_find_index_of_first_satisfying([&](field& f) {
			return &f == &resolved_field;
		});
	
	if(possible_index.has_no_value()) {
		posix::abort();
	}

	instance_field_index index = possible_index.get();

	instance_field_index_and_stack_size result {
		index,
		resolved_field.stack_size
	};

	trampoline(ref_index) = result;
	return result;
}