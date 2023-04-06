#include "class.hpp"
#include "field.hpp"

#include <class_file/constant.hpp>

#include <tuple.hpp>

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

	namespace cc = class_file::constant;

	cc::field_ref field_ref = field_ref_constant(ref_index);
	cc::name_and_type nat = name_and_type_constant(
		field_ref.name_and_type_index
	);
	cc::utf8 name = utf8_constant(nat.name_index);
	cc::utf8 desc = utf8_constant(nat.descriptor_index);

	expected<_class&, reference> possible_c
		= try_get_resolved_class(field_ref.class_index);
	
	if(possible_c.is_unexpected()) {
		return { possible_c.get_unexpected() };
	}

	_class& c = possible_c.get_expected();

	instance_field_index field_index =
		c.instance_fields().try_find_index_of(name, desc)
		.if_has_no_value([]{ posix::abort(); })
		.get();

	instance_field_index_and_stack_size result {
		.field_index = field_index,
		.stack_size = c[field_index].stack_size
	};

	trampoline(ref_index) = result;
	return result;
}