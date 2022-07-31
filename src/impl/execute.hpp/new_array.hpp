#include "array.hpp"
#include "execution/info.hpp"
#include "execution/stack_entry.hpp"

#include "abort.hpp"

#include <class_file/constant.hpp>
#include <class_file/attribute/code/instruction.hpp>

inline void new_array(
	//_class& c, TODO use trampoline
	class_file::attribute::code::instruction::new_array_type type,
	stack_entry* stack, nuint& stack_size
) {
	int32 count = stack[--stack_size].get<jint>();
	reference ref;

	using namespace class_file::attribute::code::instruction;

	switch (type) {
		case new_array_type::_bool:    ref = create_bool_array(count);   break;
		case new_array_type::_char:    ref = create_char_array(count);   break;
		case new_array_type::_float:   ref = create_float_array(count);  break;
		case new_array_type::_double:  ref = create_double_array(count); break;
		case new_array_type::_byte:    ref = create_byte_array(count);   break;
		case new_array_type::_short:   ref = create_short_array(count);  break;
		case new_array_type::_int:     ref = create_int_array(count);    break;
		case new_array_type::_long:    ref = create_long_array(count);   break;
		default:
			fputs("unknown type of array", stderr);
			abort();
	}

	if(info) {
		char type_ch;

		switch (type) {
			case new_array_type::_bool:    type_ch = 'Z'; break;
			case new_array_type::_char:    type_ch = 'C'; break;
			case new_array_type::_float:   type_ch = 'F'; break;
			case new_array_type::_double:  type_ch = 'D'; break;
			case new_array_type::_byte:    type_ch = 'B'; break;
			case new_array_type::_short:   type_ch = 'S'; break;
			case new_array_type::_int:     type_ch = 'I'; break;
			case new_array_type::_long:    type_ch = 'J'; break;
		}
		tabs(); fputs("new_array ", stderr);
		fputc((uint8) type_ch, stderr);
		fputc('\n', stderr);
	}

	stack[stack_size++] = move(ref);
}