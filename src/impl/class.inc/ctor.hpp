#include "class.hpp"

inline c::c(
	constants&& constants,
	bootstrap_methods&& bootstrap_methods,
	posix::memory<> bytes,
	class_file::access_flags access_flags,
	class_file::constant::utf8 name,
	posix::memory<utf8::unit> descriptor,
	class_file::constant::utf8 source_file,
	optional<c&> super_class,
	posix::memory<c*> declared_interfaces,
	posix::memory<static_field> declared_static_fields,
	posix::memory<instance_field> declared_instance_fields,
	posix::memory<static_method> declared_static_methods,
	posix::memory<instance_method> declared_instance_methods,
	optional<method> initialisation_method,
	is_array_class is_array,
	is_primitive_class is_primitive,
	reference defining_loader
) :
	::constants          { move(constants)              },
	::trampolines        { (uint16) ::constants::size() },
	::bootstrap_methods  { move(bootstrap_methods)      },
	super_               { super_class                  },
	bytes_               { move(bytes)                  },
	access_flags_        { access_flags                 },
	name_                { name                         },
	descriptor_          { move(descriptor)             },
	source_file_         { source_file                  },
	declared_interfaces_ { move(declared_interfaces)    },
	declared_static_fields_ { [&] {
		for(static_field& f : declared_static_fields.as_span()) {
			f.class_ = *this;
		}
		return move(declared_static_fields);
	}()},
	declared_instance_fields_ { [&] {
		for(instance_field& f : declared_instance_fields.as_span()) {
			f.class_ = *this;
		}
		return move(declared_instance_fields);
	}()},
	declared_static_methods_ { [&] {
		for(static_method& m : declared_static_methods.as_span()) {
			m.class_ = *this;
		}
		return move(declared_static_methods);
	}()},
	declared_instance_methods_ { [&] {
		for(instance_method& m : declared_instance_methods.as_span()) {
			m.class_ = *this;
		}
		return move(declared_instance_methods);
	}()},
	instance_fields_ { [&] {
		nuint count  = declared_instance_fields_.size();
		if(has_super()) {
			count += range_size(super().instance_fields());
		}
		::list fields = posix::allocate<instance_field*>(count);

		if(has_super()) {
			for(instance_field& f : super().instance_fields()) {
				fields.emplace_back<instance_field*>(&f);
			}
		}
		for(instance_field& f : declared_instance_fields_.as_span()) {
			fields.emplace_back<instance_field*>(&f);
		}
		return move(fields.storage_range());
	}()},
	instance_methods_ { [&] {
		nuint count = range_size(this->declared_instance_methods());
		if(has_super()) {
			// increase instance methods count if super instance method
			// isn't overrided by any declared instance method
			for(instance_method& m : super().instance_methods()) {
				if(
					this->declared_instance_methods()
					.try_find(m.name(), m.descriptor()).has_no_value()
				) {
					++count;
				}
			}
		}
		::list methods = posix::allocate<instance_method*>(count);

		if(has_super()) {
			for(instance_method& m : super().instance_methods()) {
				methods.emplace_back(&m);
			}
		}
		for(instance_method& m : this->declared_instance_methods()) {
			auto index_of_overriden = find_by_name_and_descriptor_view {
				methods.dereference_view()
			}.try_find_index_of(m.name(), m.descriptor());
			// declared instance method overrides super instance method
			if(index_of_overriden) {
				uint16 index = uint16{ index_of_overriden.get() };
				methods.emplace_at(index, &m);
			}
			else {
				methods.emplace_back(&m);
			}
		}
		return move(methods.storage_range());
	}()},
	initialisation_method_{ [&] {
		if(initialisation_method.has_value()) {
			initialisation_method->class_ = *this;
		}
		return move(initialisation_method);
	}()},
	instance_layout_ { [&] {
		return
			has_super() ?
			::layout {
				this->declared_instance_fields(),
				super().instance_layout()
			} :
			::layout {
				this->declared_instance_fields()
			};
	}()},
	declared_static_layout_ { this->declared_static_fields() },
	is_array_            { is_array                  },
	is_primitive_        { is_primitive              },
	defining_loader_     { move(defining_loader) },
	mutex_ { posix::create_mutex(mutex_attribute_recursive) },
	declared_static_fields_data_ {
		posix::allocate<>(
			declared_static_layout().size()
		)
	}
{
	
}