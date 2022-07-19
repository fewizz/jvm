#pragma once

#include "./info.hpp"
#include "./stack_entry.hpp"

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
	char type;
	nuint size = 0;
	switch (x.type) {
		case 4:  type = 'Z'; size = sizeof(jbool);   break;
		case 5:  type = 'C'; size = sizeof(jchar);   break;
		case 6:  type = 'F'; size = sizeof(jfloat);  break;
		case 7:  type = 'D'; size = sizeof(jdouble); break;
		case 8:  type = 'B'; size = sizeof(jbyte);   break;
		case 9:  type = 'S'; size = sizeof(jshort);  break;
		case 10: type = 'I'; size = sizeof(jint);    break;
		case 11: type = 'J'; size = sizeof(jlong);   break;
		default:
			fputs("unknown type of array", stderr);
			abort();
	}
	if(info) {
		tabs(); fputs("new_array ", stderr);
		fputc(type, stderr);
		fputc('\n', stderr);
	}
	int32 count = stack[--stack_size].get<jint>();
	_class& c0 = find_or_load_class(
		concat_view{ array{'[' }, array{ type } }
	);
	auto ref = create_object(c0);
	array_data(
		ref.object(),
		default_allocator{}.allocate_zeroed(count * size)
	);
	array_length(ref.object(), count);
	stack[stack_size++] = move(ref);
}