#include "class.hpp"

template<range Name>
optional<declared_instance_field_index> _class::
try_find_declared_instance_field_index(Name&& name) {
	uint16 index = 0;
	for(instance_field& f : instance_fields_) {
		if(equals(this->name(f), name)) {
			return declared_instance_field_index{ index };
		}
		++index;
	}
	return elements::none{};
}

template<range Name>
optional<instance_field&> _class::
try_find_declared_instance_field(Name&& name) {
	optional<declared_instance_field_index> result {
		try_find_declared_static_field_index(name)
	};
	if(result.has_value()) {
		return declared_instance_fields()[result.value()];
	}
	return elements::none{};
}

template<range Name>
declared_instance_field_index _class::
find_declared_instance_field_index(Name&& name) {
	optional<declared_instance_field_index> result {
		try_find_declared_instance_field_index(name)
	};
	if(result.has_value()) {
		return result.value();
	}
	fputs("couldn't find declared instance field index for", stderr);
	fwrite(name.data(), 1, name.size(), stderr);
	abort();
}

template<range Name>
instance_field& _class::
find_declared_instance_field(Name&& name) {
	optional<instance_field&> result {
		try_find_declared_instance_field(name)
	};
	if(result.has_value()) {
		return result.value();
	}
	fputs("couldn't find declared instance field ", stderr);
	fwrite(name.data(), 1, name.size(), stderr);
	abort();
}