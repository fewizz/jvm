#pragma once

#include "../class.hpp"
#include "../field_value.hpp"
#include "../objects.hpp"
#include "class/file/code/instruction.hpp"
#include <core/c_string.hpp>
#include <core/concat.hpp>

template<range Name>
static inline _class& find_or_load(Name name);

inline void new_array(
	//_class& c, TODO use trampoline
	class_file::code::instruction::new_array x,
	stack_entry* stack, nuint& stack_size
) {
	c_string<c_string_type::known_size> name;
	nuint size = 0;
	switch (x.type) {
		case 4:  name = "boolean"; size = sizeof(jbool);   break;
		case 5:  name = "char";    size = sizeof(jchar);   break;
		case 6:  name = "float";   size = sizeof(jfloat);  break;
		case 7:  name = "double";  size = sizeof(jdouble); break;
		case 8:  name = "byte";    size = sizeof(jbyte);   break;
		case 9:  name = "short";   size = sizeof(jshort);  break;
		case 10: name = "int";     size = sizeof(jint);    break;
		case 11: name = "long";    size = sizeof(jlong);   break;
		default:
			fputs("unknown type of array", stderr);
			abort();
	}
	if(info) {
		tabs(); fputs("new_array ", stderr);
		fwrite(name.data(), 1, name.size(), stderr);
		fputc('\n', stderr);
	}
	int32 count = stack[--stack_size].get<int32>();
	_class& c0 = find_or_load(
		concat_view{ name, array{'[', ']'} }
	);
	auto ref = objects.find_free(c0);
	ref.object().values()[0] = field_value {
		jlong {
			(int64) default_allocator{}.allocate_zeroed(
				count * size
			)
		}
	};
	ref.object().values()[1] = jint{ count };
	stack[stack_size++] = move(ref);
}