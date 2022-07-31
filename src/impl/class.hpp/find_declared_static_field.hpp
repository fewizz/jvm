#include "class.hpp"

template<range Name, range Descriptor>
optional<static_field&> _class::
try_find_declared_static_field(Name&& name, Descriptor&& descriptor) {
	for(static_field& f : static_fields_) {
		if(
			equals(this->name(f), name) &&
			equals(this->descriptor(f), descriptor)
		) return { f };
	}
	return elements::none{};
}

template<range Name, range Descriptor>
inline static_field& _class::
find_declared_static_field(Name&& name, Descriptor&& descriptor) {
	optional<static_field&> result {
		try_find_declared_static_field(name, descriptor)
	};
	if(!result.has_value()) {
		fputs("couldn't find static field", stderr); abort();
	}
	return result.value();
}