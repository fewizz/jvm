#include "decl/class.hpp"
#include "decl/class/resolve_class.hpp"

inline expected<_class&, reference> _class::try_get_resolved_class(
	class_file::constant::class_index class_index
) {
	mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(auto& t = trampoline(class_index); t.has_value()) {
		if(!t.is_same_as<_class&>()) {
			posix::abort();
		}
		return t.get_same_as<_class&>();
	}

	class_file::constant::_class cc = class_constant(class_index);
	class_file::constant::utf8 name = utf8_constant(cc.name_index);
	// note, _class& d = *this;
	expected<_class&, reference> possible_c = try_resolve_class(*this, name);
	if(possible_c.is_unexpected()) {
		return { possible_c.get_unexpected() };
	}

	_class& c = possible_c.get_expected();
	trampoline(class_index) = c;
	return c;
}