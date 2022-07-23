#pragma once

#include "execution/info.hpp"
#include "execution/stack_entry.hpp"
#include "array.hpp"
#include "class/decl.hpp"
#include "field/value.hpp"
#include "object/create.hpp"
#include "classes/find_or_load.hpp"

#include <class/file/attribute/code/instruction.hpp>

#include <core/c_string.hpp>
#include <core/concat.hpp>
#include <core/array.hpp>


inline void new_array(
	//_class& c, TODO use trampoline
	class_file::attribute::code::instruction::new_array x,
	stack_entry* stack, nuint& stack_size
) {
	int32 count = stack[--stack_size].get<jint>();
	reference ref;

	switch (x.type) {
		case 4:  ref = create_bool_array(count);   break;
		case 5:  ref = create_char_array(count);   break;
		case 6:  ref = create_float_array(count);  break;
		case 7:  ref = create_double_array(count); break;
		case 8:  ref = create_byte_array(count);   break;
		case 9:  ref = create_short_array(count);  break;
		case 10: ref = create_int_array(count);    break;
		case 11: ref = create_long_array(count);   break;
		default:
			fputs("unknown type of array", stderr);
			abort();
	}

	if(info) {
		char type;

		switch (x.type) {
			case 4:  type = 'Z'; break;
			case 5:  type = 'C'; break;
			case 6:  type = 'F'; break;
			case 7:  type = 'D'; break;
			case 8:  type = 'B'; break;
			case 9:  type = 'S'; break;
			case 10: type = 'I'; break;
			case 11: type = 'J'; break;
		}
		tabs(); fputs("new_array ", stderr);
		fputc(type, stderr);
		fputc('\n', stderr);
	}

	stack[stack_size++] = move(ref);
}