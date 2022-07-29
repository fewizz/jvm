#pragma once

#include "./constant_index.hpp"
#include "execution/info.hpp"
#include "execution/stack_entry.hpp"
#include "class.hpp"

inline void ldc(
	constant_index const_index, _class& c,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		tabs(); fputs("ldc ", stderr);
		fprintf(stderr, "%hhd\n", (uint8) const_index);
	}
	namespace cc = class_file::constant;
	const_pool_entry constatnt = c.constant(const_index);
	if(constatnt.is<cc::_int>()) {
		stack[stack_size++] = jint {
			constatnt.get<cc::_int>().value
		};
	} else
	if(constatnt.is<cc::_float>()) {
		stack[stack_size++] = jfloat {
			constatnt.get<cc::_float>().value
		};
	} else
	if(constatnt.is<cc::string>()) {
		stack[stack_size++] = c.get_string(const_index);
	} else
	if(constatnt.is<cc::_class>()) {
		stack[stack_size++] = c.get_class(const_index).reference();
	}
	else {
		fputs("unknown constant", stderr); abort();
	}
}

inline void ldc_w(
	wide_constant_index const_index, _class& c,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		tabs(); fputs("ldc_w ", stderr);
		fprintf(stderr, "%hd\n", (uint16) const_index);
	}
	namespace cc = class_file::constant;
	const_pool_entry constatnt = c.constant(const_index);
	if(constatnt.is<cc::_int>()) {
		stack[stack_size++] = jint {
			constatnt.get<cc::_int>().value
		};
	} else
	if(constatnt.is<cc::_float>()) {
		stack[stack_size++] = jfloat {
			constatnt.get<cc::_float>().value
		};
	} else
	if(constatnt.is<cc::string>()) {
		stack[stack_size++] = c.get_string(const_index);
	} else
	if(constatnt.is<cc::_class>()) {
		stack[stack_size++] = c.get_class(const_index).reference();
	}
	else {
		fputs("unknown constant", stderr); abort();
	}
}

inline void ldc_2_w(
	wide_constant_index const_index, _class& c,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		tabs(); fputs("ldc_2_w ", stderr);
		fprintf(stderr, "%hd\n", (uint16) const_index);
	}
	namespace cc = class_file::constant;
	const_pool_entry constatnt = c.constant(const_index);
	if(constatnt.is<cc::_long>()) {
		stack[stack_size++] = jlong {
			constatnt.get<cc::_long>().value
		};
	} else
	if(constatnt.is<cc::_double>()) {
		stack[stack_size++] = jdouble {
			constatnt.get<cc::_double>().value
		};
	}
	else {
		fputs("unknown constant", stderr); abort();
	}
}