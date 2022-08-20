#include "class.hpp"

inline _class::_class(
	constants&&              constants,
	bootstrap_methods&&      bootstrap_methods,
	memory_span              bytes,
	class_file::access_flags access_flags,
	this_class_name          this_name,
	optional<_class&>        super_class,
	::declared_interfaces&&  declared_interfaces,
	::declared_fields&&      declared_fields,
	::declared_methods&&     declared_methods,
	is_array_class           is_array,
	is_primitive_class       is_primitive
) :
	::constants          { move(constants)           },
	::trampolines        { constants_count()         },
	::bootstrap_methods  { move(bootstrap_methods)   },
	bytes_               { bytes                     },
	access_flags_        { access_flags              },
	this_name_           { this_name                 },
	super_               { super_class               },
	declared_interfaces_ { move(declared_interfaces) },
	declared_fields_     { move(declared_fields)     },
	declared_methods_    { move(declared_methods)    },
	is_array_            { is_array                  },
	is_primitive_        { is_primitive              }
{
	for(field& f : this->declared_fields()) {
		f.class_ = *this;
	}
	for(method& m : this->declared_methods()) {
		m.class_ = *this;
	}

	// declared static members
	nuint declared_static_fields_count  = 0;
	nuint declared_static_methods_count = 0;
	for(field& f : this->declared_fields()) {
		if(f.access_flags()._static()) {
			++declared_static_fields_count;
		}
	}
	for(method& m : this->declared_methods()) {
		if(m.access_flags()._static()) {
			++declared_static_methods_count;
		}
	}

	declared_static_fields_ = {
		allocate_for<field*>(declared_static_fields_count)
	};
	declared_static_methods_ = {
		allocate_for<method*>(declared_static_methods_count)
	};

	// declared instance members
	nuint declared_instance_fields_count =
		this->declared_fields().size() - declared_static_fields_count;
	nuint declared_instance_methods_count =
		this->declared_methods().size() - declared_static_methods_count;

	declared_instance_fields_ = {
		allocate_for<field*>(declared_instance_fields_count)
	};
	declared_instance_methods_ = {
		allocate_for<method*>(declared_instance_methods_count)
	};

	// filling up declared static and instance members
	for(field& f : this->declared_fields()) {
		if(f.access_flags()._static()) {
			declared_static_fields_.emplace_back(f);
		}
		else {
			declared_instance_fields_.emplace_back(f);
		}
	}
	for(method& m : this->declared_methods()) {
		if(m.access_flags()._static()) {
			declared_static_methods_.emplace_back(m);
		}
		else {
			declared_instance_methods_.emplace_back(m);
		}
	}

	// instance members
	nuint instance_fields_count  = declared_instance_fields_count;
	nuint instance_methods_count = declared_instance_methods_count;
	if(has_super()) {
		instance_fields_count  += super().instance_fields().size();
		// increase instance methods count if super instance method
		// isn't overrided by any declared instance method
		for(method& m : super().instance_methods()) {
			if(!declared_instance_methods_.try_find(m.name(), m.descriptor())) {
				++instance_methods_count;
			}
		}
	}

	instance_fields_ = {
		allocate_for<field*>(instance_fields_count)
	};
	instance_methods_ = {
		allocate_for<method*>(instance_methods_count)
	};

	if(has_super()) {
		for(field& f : super().instance_fields()) {
			instance_fields_.emplace_back(f);
		}
		for(method& m : super().instance_methods()) {
			instance_methods_.emplace_back(m);
		}
	}
	for(field& f : declared_instance_fields_) {
		instance_fields_.emplace_back(f);
	}
	for(method& m : declared_instance_methods_) {
		auto index_of_overriden =
			instance_methods_.try_find_index_of(m.name(), m.descriptor());
		// declared instance method overrides super instance method
		if(index_of_overriden) {
			instance_methods_.emplace_at(index_of_overriden.value(), m);
		}
		else {
			instance_methods_.emplace_back(m);
		}
	}

	declared_static_fields_values_ = {
		allocate_for<field_value>(declared_static_fields_count)
	};
}