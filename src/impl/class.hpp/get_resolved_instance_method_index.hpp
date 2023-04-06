#include "decl/class.hpp"
#include "decl/method.hpp"

#include <class_file/constant.hpp>

#include <loop_action.hpp>

inline expected<method&, reference> _class::try_get_resolved_method(
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
	expected<method&, reference> possible_m = try_resolve_method(method_ref);
	if(possible_m.is_unexpected()) {
		return { possible_m.get_unexpected() };
	}

	method& m = possible_m.get_expected();

	trampoline(ref_index) = m;
	return m;
}