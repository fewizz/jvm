#include "decl/class.hpp"

const field& _class::operator [] (instance_field_index index) const {
	return instance_fields()[index];
}

      field& _class::operator [] (instance_field_index index) {
	return instance_fields()[index];
}

const method& _class::operator [] (instance_method_index index) const {
	return instance_methods()[index];
}

      method& _class::operator [] (instance_method_index index) {
	return instance_methods()[index];
}

const method& _class::operator [] (declared_instance_method_index index) const {
	return declared_instance_methods()[index];
}

      method& _class::operator [] (declared_instance_method_index index) {
	return declared_instance_methods()[index];
}

const method& _class::operator [] (declared_static_method_index index) const {
	return declared_static_methods()[index];
}

      method& _class::operator [] (declared_static_method_index index) {
	return declared_static_methods()[index];
}