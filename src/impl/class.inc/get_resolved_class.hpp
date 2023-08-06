#include "decl/class.hpp"
#include "decl/class/resolve_class.hpp"

inline expected<c&, reference> c::try_get_resolved_class(
	class_file::constant::class_index class_index
) {
	mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(auto& t = trampoline(class_index); t.has_value()) {
		if(!t.is_same_as<c&>()) {
			posix::abort();
		}
		return t.get_same_as<c&>();
	}

	class_file::constant::_class cc = (*this)[class_index];
	class_file::constant::utf8 name = (*this)[cc.name_index];
	// note, c& d = *this;
	expected<c&, reference> possible_c = try_resolve_class(*this, name);
	if(possible_c.is_unexpected()) {
		return { possible_c.get_unexpected() };
	}

	c& c = possible_c.get_expected();
	trampoline(class_index) = c;
	return c;
}