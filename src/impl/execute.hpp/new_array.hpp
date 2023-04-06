#include "array.hpp"
#include "execution/info.hpp"
#include "execution/stack.hpp"

#include <class_file/constant.hpp>
#include <class_file/attribute/code/instruction.hpp>

[[nodiscard]] inline optional<reference> try_new_array(
	class_file::attribute::code::instruction::new_array_type type
) {
	int32 size = stack.pop_back<int32>();

	using namespace class_file::attribute::code::instruction;

	expected<reference, reference> possible_ref = [&](){
		switch (type) {
			case new_array_type::_bool:
				return try_create_bool_array(size);   break;
			case new_array_type::_char:
				return try_create_char_array(size);   break;
			case new_array_type::_float:
				return try_create_float_array(size);  break;
			case new_array_type::_double:
				return try_create_double_array(size); break;
			case new_array_type::_byte:
				return try_create_byte_array(size);   break;
			case new_array_type::_short:
				return try_create_short_array(size);  break;
			case new_array_type::_int:
				return try_create_int_array(size);    break;
			case new_array_type::_long:
				return try_create_long_array(size);   break;
			default:
				print::err("unknown array type\n");
				posix::abort();
		}
	}();

	if(possible_ref.is_unexpected()) {
		return { move(possible_ref.get_unexpected()) };
	}

	reference ref = move(possible_ref.get_expected());

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
		print::out("new_array ", (uint8) type_ch, " of size ", size, "\n");
	}

	stack.emplace_back(move(ref));

	return {};
}