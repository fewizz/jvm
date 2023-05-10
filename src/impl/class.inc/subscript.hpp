#include "decl/class.hpp"

const instance_field& c::operator [] (instance_field_index index) const {
	return instance_fields()[index];
}

      instance_field& c::operator [] (instance_field_index index) {
	return instance_fields()[index];
}

const static_field& c::operator [] (declared_static_field_index index) const {
	return declared_static_fields()[index];
}

      static_field& c::operator [] (declared_static_field_index index) {
	return declared_static_fields()[index];
}

const instance_method& c::operator [] (instance_method_index index) const {
	return instance_methods()[index];
}

      instance_method& c::operator [] (instance_method_index index) {
	return instance_methods()[index];
}

const instance_method&
c::operator [] (declared_instance_method_index index) const {
	return declared_instance_methods()[index];
}
      instance_method&
c::operator [] (declared_instance_method_index index)       {
	return declared_instance_methods()[index];
}

const static_method&
c::operator [] (declared_static_method_index index) const {
	return declared_static_methods()[index];
}
      static_method&
c::operator [] (declared_static_method_index index)       {
	return declared_static_methods()[index];
}