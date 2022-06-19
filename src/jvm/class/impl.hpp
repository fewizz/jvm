#pragma once

#include "declaration.hpp"
#include "../object/create.hpp"
#include "../classes/find_or_load.hpp"

_class::_class(
	const_pool&& const_pool,
	span<uint8> data, class_file::access_flags access_flags,
	::this_class_index this_class_index, ::super_class_index super_class_index,
	interfaces_indices_container&& interfaces,
	fields_container&& fields, methods_container&& methods
) :
	::const_pool{ move(const_pool) },
	::trampoline_pool{ ::const_pool::size() },
	data_{ data },
	access_flags_{ access_flags },
	this_class_index_{ this_class_index },
	super_class_index_{ super_class_index },
	interfaces_{ move(interfaces) },
	fields_{ move(fields) },
	methods_{ move(methods) }
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
		field& f = f0.get<field>();
		if(
			equals(this->name(f), name) &&
			equals(this->descriptor(f), descriptor)
		) return { f };
	}
	return elements::none{};
}

template<range Name>
optional<field&> _class::try_find_field(Name name) {
	for(auto& f0 : fields_) {
		field& f = f0.get<field>();
		if(equals(this->name(f), name)) {
			return { f };
		}
	}
	return elements::none{};
}

template<range Name, range Descriptor>
optional<method&> _class::try_find_method(
	Name name, Descriptor descriptor
) {
	for(method& m : methods_) {
		if(
			equals(this->name(m), name) &&
			equals(this->descriptor(m), descriptor)
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
	for(method& m : methods_) {
		if(equals(this->name(m), name)) {
			return { m };
		}
	}
	return elements::none{};
}

template<range Name, range Descriptor>
optional<field_index>
_class::try_find_instance_field_index(
	Name name, Descriptor descriptor, uint16 index
) {
	if(has_super_class()) {
		super_class().try_find_instance_field_index(name, descriptor, index);
	}
	for(auto& f0 : fields_) {
		if(f0.is<field>()) {
			field& f = f0.get<field>();
			if(
				equals(this->name(f), name) &&
				equals(this->descriptor(f), descriptor)
			) {
				return { field_index{ index } };
			}
			++index;
		}
	}
	return elements::none{};
}

void _class::initialise_if_need() {
	if(
		initialisation_state_ == initialised ||
		initialisation_state_ == pending
	) {
		return;
	}

	initialisation_state_ = pending;

	auto clinit = try_find_method(c_string{ "<clinit>" });
	if(clinit.has_value()) {
		execute(method_with_class{ clinit.value(), *this });
	}
	initialisation_state_ = initialised;

	if(has_super_class()) {
		super_class().initialise_if_need();
	}

	for(_class& i : interfaces()) {
		i.initialise_if_need();
	}
}

#include "impl/get_static_field.hpp"
#include "impl/get_static_method.hpp"
#include "impl/get_resolved_method.hpp"
#include "impl/get_class.hpp"
#include "impl/get_resolved_field.hpp"
#include "impl/for_each_maximally_specific_superinterface_method.hpp"
#include "impl/get_string.hpp"