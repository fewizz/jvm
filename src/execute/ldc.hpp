#pragma once

#include "execution/info.hpp"
#include "execution/stack_entry.hpp"
#include "class/decl.hpp"

#include <class/file/attribute/code/instruction.hpp>

inline void ldc(
	_class& c, class_file::attribute::code::instruction::ldc x,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		tabs(); fputs("ldc ", stderr);
		fprintf(stderr, "%hhd\n", x.index);
	}
	namespace cc = class_file::constant;
	const_pool_entry constatnt = c.constant(x.index);
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
		stack[stack_size++] = c.get_string(x.index);
	} else
	if(constatnt.is<cc::_class>()) {
		stack[stack_size++] = c.get_class(x.index).reference();
	}
	else {
		fputs("unknown constant", stderr); abort();
	}
}

inline void ldc_w(
	_class& c, class_file::attribute::code::instruction::ldc_w x,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		tabs(); fputs("ldc_w ", stderr);
		fprintf(stderr, "%hd\n", x.index);
	}
	namespace cc = class_file::constant;
	const_pool_entry constatnt = c.constant(x.index);
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
		stack[stack_size++] = c.get_string(x.index);
	} else
	if(constatnt.is<cc::_class>()) {
		stack[stack_size++] = c.get_class(x.index).reference();
	}
	else {
		fputs("unknown constant", stderr); abort();
	}
}

inline void ldc_2_w(
	_class& c, class_file::attribute::code::instruction::ldc_2_w x,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		tabs(); fputs("ldc_2_w ", stderr);
		fprintf(stderr, "%hd\n", x.index);
	}
	namespace cc = class_file::constant;
	const_pool_entry constatnt = c.constant(x.index);
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