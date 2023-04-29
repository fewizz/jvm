#include "decl/class.hpp"

const instance_field&
_class::operator [] (instance_field_index index) const {
	return instance_fields()[index];
}

      instance_field&
_class::operator [] (instance_field_index index) {
	return instance_fields()[index];
}

const static_field&
_class::operator [] (declared_static_field_index index) const {
	return declared_static_fields()[index];
}

      static_field&
_class::operator [] (declared_static_field_index index) {
	return declared_static_fields()[index];
}

const instance_method& _class::operator [] (instance_method_index index) const {
	return instance_methods()[index];
}

      instance_method& _class::operator [] (instance_method_index index) {
	return instance_methods()[index];
}

const instance_method&
_class::operator [] (declared_instance_method_index index) const {
	return declared_instance_methods()[index];
}
      instance_method&
_class::operator [] (declared_instance_method_index index)       {
	return declared_instance_methods()[index];
}

const static_method&
_class::operator [] (declared_static_method_index index) const {
	return declared_static_methods()[index];
}
      static_method&
_class::operator [] (declared_static_method_index index)       {
	return declared_static_methods()[index];
}