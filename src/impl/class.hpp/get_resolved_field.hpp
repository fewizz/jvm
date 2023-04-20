#include "class.hpp"
#include "field.hpp"
#include "decl/class/resolve_field.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"

#include <class_file/constant.hpp>

#include <tuple.hpp>
#include <optional.hpp>

template<typename Verifier>
inline expected<field&, reference>
_class::try_get_resolved_field(
	class_file::constant::field_ref_index ref_index, Verifier&& verifier
) {
	mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(auto& t = trampoline(ref_index); t.has_value()) {
		if(!t.is_same_as<field&>()) {
			posix::abort();
		}
		return t.get_same_as<field&>();
	}

	expected<field&, reference> possible_resolved_field
		= try_resolve_field(ref_index);
	
	if(possible_resolved_field.is_unexpected()) {
		return unexpected{ move(possible_resolved_field.get_unexpected()) };
	}

	field& resolved_field = possible_resolved_field.get_expected();

	optional<reference> possible_error = verifier(resolved_field);
	if(possible_error.has_value()) {
		return move(possible_error.get());
	}

	trampoline(ref_index) = resolved_field;
	return resolved_field;
}