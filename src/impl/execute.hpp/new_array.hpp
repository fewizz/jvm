#include "array.hpp"
#include "execution/info.hpp"
#include "execution/stack.hpp"

#include <class_file/constant.hpp>
#include <class_file/attribute/code/instruction.hpp>

inline void new_array(
	class_file::attribute::code::instruction::new_array_type type
) {
	int32 size = stack.pop_back<int32>();
	reference ref;

	using namespace class_file::attribute::code::instruction;

	switch (type) {
		case new_array_type::_bool:    ref = create_bool_array(size);   break;
		case new_array_type::_char:    ref = create_char_array(size);   break;
		case new_array_type::_float:   ref = create_float_array(size);  break;
		case new_array_type::_double:  ref = create_double_array(size); break;
		case new_array_type::_byte:    ref = create_byte_array(size);   break;
		case new_array_type::_short:   ref = create_short_array(size);  break;
		case new_array_type::_int:     ref = create_int_array(size);    break;
		case new_array_type::_long:    ref = create_long_array(size);   break;
		default:
			posix::std_err.write_from(c_string{ "unknown array type\n" });
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
		tabs();
		print("new_array ");
		print((uint8) type_ch);
		print(" of size ");
		print(size);
		print("\n");
	}

	stack.emplace_back(move(ref));
}