#include "decl/class.hpp"
#include "decl/method.hpp"

#include <class_file/constant.hpp>

#include <loop_action.hpp>

template<typename Verifier>
inline expected<method&, reference> _class::try_get_resolved_method(
	class_file::constant::method_ref_index ref_index,
	Verifier&& verifier
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

	expected<method&, reference> possible_m
		= this->try_resolve_method(ref_index);

	if(possible_m.is_unexpected()) {
		return { possible_m.get_unexpected() };
	}

	method& m = possible_m.get_expected();

	optional<reference> possible_throwable = verifier(m);
	if(possible_throwable.has_value()) {
		return unexpected{ possible_throwable.move() };
	}

	trampoline(ref_index) = m;

	return m;
}