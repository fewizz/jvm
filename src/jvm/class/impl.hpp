#pragma once

#include "member/impl.hpp"

#include "decl.hpp"
#include "../array.hpp"
#include "../object/create.hpp"
#include "../classes/find_or_load.hpp"

inline _class::_class(
	const_pool&& const_pool,
	span<uint8> data, class_file::access_flags access_flags,
	::this_class_index this_class_index, ::super_class_index super_class_index,
	interfaces_indices_container&& interfaces,
	instance_fields_container&& instance_fields,
	static_fields_container&& static_fields,
	methods_container&& methods
) :
	::const_pool{ move(const_pool) },
	::trampoline_pool{ ::const_pool::size() },
	data_{ data },
	access_flags_{ access_flags },
	this_class_index_{ this_class_index },
	super_class_index_{ super_class_index },
	interfaces_{ move(interfaces) },
	instance_fields_{ move(instance_fields) },
	static_fields_{ move(static_fields) },
	methods_{ move(methods) }
{}

inline reference _class::reference() {
	if(reference_.is_null()) {
		_class& class_class = find_or_load_class(c_string{ "java/lang/Class" });
		reference_ = create_object(class_class);
		auto class_data_location0 = class_class.try_find_instance_field_index(
			c_string{ "classData" }, c_string{ "Ljava/lang/Object;" }
		);
		if(!class_data_location0.has_value()) {
			fputs("couldn't find classData field in Class class", stderr);
			abort();
		}
		::reference long_ref {
			create_object(find_or_load_class(c_string{ "[J" }))
		};
		array_data(long_ref.object(), this);
		instance_field_index class_data_location = class_data_location0.value();
		reference_.object().values()[class_data_location] = long_ref;
	}
	return reference_;
}

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

template<range Name, range Descriptor>
static bool
try_find_instance_field_index0(
	_class& c, Name&& name, Descriptor&& descriptor, uint16& index
) {
	if(c.has_super_class()) {
		_class& super = c.super_class();
		bool result {
			try_find_instance_field_index0(super, name, descriptor, index)
		};
		if(result) {
			return true;
		}
	}
	for(instance_field& f : c.declared_instance_fields()) {
		if(
			equals(c.name(f), name) &&
			equals(c.descriptor(f), descriptor)
		) {
			return true;
		}
		++index;
	}
	return false;
}

template<range Name, range Descriptor>
optional<instance_field_index>
_class::try_find_instance_field_index(Name&& name, Descriptor&& descriptor) {
	uint16 index = 0;
	bool result {
		try_find_instance_field_index0(*this, name, descriptor, index)
	};
	if(result) {
		return instance_field_index{ index };
	}
	return elements::none{};
}

static inline optional<instance_field_with_class>
try_get_instance_field0(_class& c, uint16& index) {
	if(c.has_super_class()) {
		_class& super = c.super_class();
		optional<instance_field_with_class> result {
			try_get_instance_field0(super, index)
		};
		if(result.has_value()) {
			return result;
		}
	}
	// TODO
	uint16 declared_instance_fields_count = c.declared_instance_fields().size();
	if(index >= declared_instance_fields_count) {
		index -= declared_instance_fields_count;
		return elements::none{};
	}
	return instance_field_with_class{
		c.declared_instance_fields()[index], c
	};
}

inline optional<instance_field_with_class> _class::
try_get_instance_field(instance_field_index index) {
	return try_get_instance_field0(*this, index._);
}

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

inline void _class::initialise_if_need() {
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
}

#include "impl/get_static_field.hpp"
#include "impl/get_static_method.hpp"
#include "impl/get_resolved_method.hpp"
#include "impl/get_class.hpp"
#include "impl/get_resolved_instance_field.hpp"
#include "impl/for_each_maximally_specific_superinterface_method.hpp"
#include "impl/get_string.hpp"