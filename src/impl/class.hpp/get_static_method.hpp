#include "decl/class.hpp"
#include "decl/method.hpp"

#include <class_file/constant.hpp>

inline expected<method&, reference> _class::try_get_static_method(
	class_file::constant::method_ref_index ref_index
) {
	mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(auto& t = trampoline(ref_index); t.has_value()) {
		if(!t.is_same_as<method&>()) {
			posix::abort();
		}
		return t.get_same_as<method&>();
	}

	namespace cc = class_file::constant;

	cc::method_ref method_ref = method_ref_constant(ref_index);
	cc::name_and_type nat {
		name_and_type_constant(method_ref.name_and_type_index)
	};
	cc::utf8 name = utf8_constant(nat.name_index);
	cc::utf8 desc = utf8_constant(nat.descriptor_index);

	expected<_class&, reference> possible_c
		= try_get_resolved_class(method_ref.class_index);

	if(possible_c.is_unexpected()) {
		return { possible_c.get_unexpected() };
	}

	_class& c = possible_c.get_expected();

	method& m = c.declared_static_methods().find(name, desc);
	/* "On successful resolution of the method, the class or interface that
	    declared the resolved method is initialized (§5.5) if that class or
	    interface has not already been initialized." */
	optional<reference> possible_throwable = c.try_initialise_if_need();
	if(possible_throwable.has_value()) {
		return move(possible_throwable.get());
	}
	trampoline(ref_index) = m;
	return m;
}