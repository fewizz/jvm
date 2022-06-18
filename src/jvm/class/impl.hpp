#pragma once

#include "declaration.hpp"
#include "../object/create.hpp"
#include "../classes/find_or_load.hpp"

_class::_class(const_pool&& const_pool) :
	::const_pool{ move(const_pool) },
	::trampoline_pool{ constants_count() }
{}

object& _class::object() {
	if(reference_.is_null()) {
		_class& class_class = find_or_load_class(c_string{ "java/lang/Class" });
		reference_ = create_object(class_class);
		auto& values = reference_.object().values();
		auto class_data_location0 = class_class.try_find_instance_field_index(
			c_string{ "classData" }, c_string{ "Ljava/lang/Object;" }
		);
		if(!class_data_location0.has_value()) {
			fputs("couldn't find classData field in Class class", stderr);
			abort();
		}
		reference long_ref {
			create_object(find_or_load_class(c_string{ "java/lang/Long" }))
		};
		auto class_data_location = class_data_location0.value();
		values[class_data_location].get<reference>() = long_ref;
	}
	return reference_.object();
}

template<range Name, range Descriptor>
optional<field&> _class::try_find_field(Name name, Descriptor descriptor) {
	for(auto& f0 : fields_) {
		auto& f = f0.get<field>();
		if(
			equals(f.name(), name) &&
			equals(f.descriptor(), descriptor)
		) return { f };
	}
	return elements::none{};
}

template<range Name, range Descriptor>
optional<field_index>
_class::try_find_instance_field_index(Name name, Descriptor descriptor) {
	uint16 index = 0;
	for(auto& f : instance_fields_) {
		if(
			equals(f.name(), name) &&
			equals(f.descriptor(), descriptor)
		) return field_index{ index };
		++index;
	}
	return elements::none{};
}

template<range Name>
optional<field&> _class::try_find_field(Name name) {
	for(auto& f : fields_) {
		if(equals(f.get<field>().name(), name)) {
			return { f.get<field>() };
		}
	}
	return elements::none{};
}

template<range Name, range Descriptor>
optional<method&> _class::try_find_method(
	Name name, Descriptor descriptor
) {
	for(auto& m : methods_) {
		if(
			equals(m.name(), name) &&
			equals(m.descriptor(), descriptor)
		) return { m };
	}
	return elements::none{};
}

template<range Name, range Descriptor>
const optional<method&> _class::try_find_method(
	Name name, Descriptor descriptor
) const {
	return ((_class*)this)->try_find_method(
		forward<Name>(name), forward<Descriptor>(descriptor)
	);
}

template<range Name>
optional<method&> _class::try_find_method(Name name) {
	for(auto& m : methods_) {
		if(equals(m.name(), name)) {
			return { m };
		}
	}
	return elements::none{};
}

#include "impl/get_field.hpp"
#include "impl/get_method.hpp"
#include "impl/get_resolved_method.hpp"
#include "impl/get_class.hpp"
#include "impl/get_resolved_field.hpp"
#include "impl/for_each_maximally_specific_superinterface_method.hpp"
#include "impl/get_string.hpp"