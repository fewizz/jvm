#pragma once

#include "info.hpp"
#include "stack_entry.hpp"
#include "../class/declaration.hpp"
#include "class/file/code/instruction.hpp"

inline void ldc(
	_class& c, class_file::code::instruction::ldc x,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		tabs(); fputs("ldc ", stderr);
		fprintf(stderr, "%hhd\n", x.index);
	}
	namespace cc = class_file::constant;
	const_pool_entry constatnt = c.constant(x.index);
	if(constatnt.is<cc::int32>()) {
		stack[stack_size++] = jint {
			constatnt.get<cc::int32>().value
		};
	} else
	if(constatnt.is<cc::float32>()) {
		stack[stack_size++] = jfloat {
			constatnt.get<cc::float32>().value
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
	_class& c, class_file::code::instruction::ldc_w x,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		tabs(); fputs("ldc_w ", stderr);
		fprintf(stderr, "%hd\n", x.index);
	}
	namespace cc = class_file::constant;
	const_pool_entry constatnt = c.constant(x.index);
	if(constatnt.is<cc::int32>()) {
		stack[stack_size++] = jint {
			constatnt.get<cc::int32>().value
		};
	} else
	if(constatnt.is<cc::float32>()) {
		stack[stack_size++] = jfloat {
			constatnt.get<cc::float32>().value
		};
	} else
	if(constatnt.is<cc::string>()) {
		stack[stack_size++] = c.get_string(x.index);
	}
	else {
		fputs("unknown constant", stderr); abort();
	}
}

inline void ldc_2_w(
	_class& c, class_file::code::instruction::ldc_2_w x,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		tabs(); fputs("ldc_2_w ", stderr);
		fprintf(stderr, "%hd\n", x.index);
	}
	namespace cc = class_file::constant;
	const_pool_entry constatnt = c.constant(x.index);
	if(constatnt.is<cc::int64>()) {
		stack[stack_size++] = jlong {
			constatnt.get<cc::int64>().value
		};
	} else
	if(constatnt.is<cc::float64>()) {
		stack[stack_size++] = jdouble {
			constatnt.get<cc::float64>().value
		};
	}
	else {
		fputs("unknown constant", stderr); abort();
	}
}