#include "decl/class.hpp"
#include "decl/class/resolve_class.hpp"

inline _class& _class::get_resolved_class(
	class_file::constant::class_index class_index
) {
	if(auto& t = trampoline(class_index); t.has_value()) {
		if(!t.is_same_as<_class&>()) {
			abort();
		}
		return t.get_same_as<_class&>();
	}

	class_file::constant::_class cc = class_constant(class_index);
	class_file::constant::utf8 name = utf8_constant(cc.name_index);
	_class& c = resolve_class(*this, name);
	trampoline(class_index) = c;
	return c;
}