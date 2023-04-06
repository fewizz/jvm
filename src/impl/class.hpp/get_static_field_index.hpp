#include "decl/class.hpp"
#include "decl/field.hpp"

#include <class_file/constant.hpp>

inline expected<class_and_declared_static_field_index, reference>
_class::try_get_static_field_index(
	class_file::constant::field_ref_index ref_index
) {
	mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(auto& t = trampoline(ref_index); t.has_value()) {
		if(!t.is_same_as<class_and_declared_static_field_index>()) {
			posix::abort();
		}
		return t.get_same_as<class_and_declared_static_field_index>();
	}

	namespace cc = class_file::constant;

	cc::field_ref field_ref = field_ref_constant(ref_index);
	cc::name_and_type nat {
		name_and_type_constant(field_ref.name_and_type_index)
	};
	cc::utf8 name = utf8_constant(nat.name_index);
	cc::utf8 desc = utf8_constant(nat.descriptor_index);

	expected<_class&, reference> possible_c
		= try_get_resolved_class(field_ref.class_index);
	
	if(possible_c.is_unexpected()) {
		return { possible_c.get_unexpected() };
	}

	_class& c = possible_c.get_expected();

	optional<reference> possible_throwable = c.try_initialise_if_need();
	if(possible_throwable.has_value()) {
		return move(possible_throwable.get());
	}

	declared_static_field_index index =
		c.declared_static_fields().try_find_index_of(name, desc)
		.if_has_no_value(posix::abort)
		.get();

	class_and_declared_static_field_index result {
		._class = c,
		.field_index = index
	};

	trampoline(ref_index) = result;
	return result;
}