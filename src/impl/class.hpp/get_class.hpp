#include "decl/class.hpp"
#include "decl/classes.hpp"

inline _class& _class::get_class(
	class_file::constant::class_index class_index
) {
	if(auto& t = trampoline(class_index); !t.is<elements::none>()) {
		if(!t.is<_class&>()) {
			abort();
		}
		return t.get<_class&>();
	}

	class_file::constant::_class cc = class_constant(class_index);
	class_file::constant::utf8 name = utf8_constant(cc.name_index);
	_class& c = classes.find_or_load(name);
	trampoline(class_index) = c;
	return c;
}