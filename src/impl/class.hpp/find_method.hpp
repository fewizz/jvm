#include "class.hpp"

template<range Name, range Descriptor>
optional<method&> _class::
try_find_method(Name&& name, Descriptor&& descriptor) {
	for(method& m : methods_) {
		if(
			equals(this->name(m), name) &&
			equals(this->descriptor(m), descriptor)
		) return { m };
	}
	return elements::none{};
}

template<range Name>
optional<method&> _class::
try_find_method(Name&& name) {
	for(method& m : methods_) {
		if(equals(this->name(m), name)) {
			return { m };
		}
	}
	return elements::none{};
}