#pragma once

#include "../../code/instruction.hpp"

#include <core/meta/elements/of.hpp>
#include <core/read.hpp>

namespace class_file::attribute::code {

	enum class reader_stage {
		max_stack,
		max_locals,
		code,
		exception_table,
		attributes
	};

	template<typename Iterator, reader_stage Stage = reader_stage::max_stack>
	struct reader {
		Iterator src;

		elements::of<reader<Iterator, reader_stage::max_locals>, uint16>
		operator () () const
		requires (Stage == reader_stage::max_stack) {
			Iterator cpy = src;
			uint16 max_stack = read<uint16>(cpy);
			return { { cpy }, { max_stack } };
		}

		elements::of<reader<Iterator, reader_stage::code>, uint16>
		operator () () const
		requires (Stage == reader_stage::max_locals) {
			Iterator cpy = src;
			uint16 max_locals = read<uint16>(cpy);
			return { { cpy }, { max_locals } };
		}

		template<typename Handler>
		reader<Iterator, reader_stage::exception_table>
		operator () (Handler&& handler) const
		requires (Stage == reader_stage::code) {
			using namespace class_file::code::instruction;

			Iterator cpy = src;
			uint32 length = read<uint32>(cpy);

			while(cpy - (src + 4) < length) {
				uint8 first = read<uint8>(cpy);

				switch (first) {
					case 0: handler(nop{}); break;

					case 2: handler(i_const_m1{}); break;
					case 3: handler(i_const_0{}); break;
					case 4: handler(i_const_1{}); break;
					case 5: handler(i_const_2{}); break;
					case 6: handler(i_const_3{}); break;
					case 7: handler(i_const_4{}); break;
					case 8: handler(i_const_5{}); break;

					case 18: {
						uint8 index = read<uint8>(cpy);
						handler(ldc{ index }); break;
					}
					case 21: {
						uint8 index = read<uint8>(cpy);
						handler(i_load{ index }); break;
					}
					case 26: handler(i_load_0{}); break;
					case 27: handler(i_load_1{}); break;
					case 28: handler(i_load_2{}); break;
					case 29: handler(i_load_3{}); break;
					case 42: handler(a_load_0{}); break;
					case 43: handler(a_load_1{}); break;
					case 44: handler(a_load_2{}); break;
					case 45: handler(a_load_3{}); break;

					case 54: {
						uint8 index = read<uint8>(cpy);
						handler(i_store{ index }); break;
					}
					case 59: handler(i_store_0{}); break;
					case 60: handler(i_store_1{}); break;
					case 61: handler(i_store_2{}); break;
					case 62: handler(i_store_3{}); break;
					case 75: handler(a_store_0{}); break;
					case 76: handler(a_store_1{}); break;
					case 77: handler(a_store_2{}); break;
					case 78: handler(a_store_3{}); break;

					case 87: handler(pop{}); break;
					case 89: handler(dup{}); break;

					case 153: {
						uint16 branch = read<uint16>(cpy);
						handler(if_eq{ branch }); break;
					}
					case 154: {
						uint16 branch = read<uint16>(cpy);
						handler(if_ne{ branch }); break;
					}
					case 155: {
						uint16 branch = read<uint16>(cpy);
						handler(if_lt{ branch }); break;
					}
					case 156: {
						uint16 branch = read<uint16>(cpy);
						handler(if_ge{ branch }); break;
					}
					case 157: {
						uint16 branch = read<uint16>(cpy);
						handler(if_gt{ branch }); break;
					}
					case 158: {
						uint16 branch = read<uint16>(cpy);
						handler(if_le{ branch }); break;
					}
					case 165: {
						uint16 branch = read<uint16>(cpy);
						handler(if_a_cmp_eq{ branch }); break;
					}
					case 166: {
						uint16 branch = read<uint16>(cpy);
						handler(if_a_cmp_ne{ branch }); break;
					}
					case 167: {
						uint16 branch = read<uint16>(cpy);
						handler(go_to{ branch }); break;
					}

					case 177: handler(rtrn{}); break;

					case 178: {
						uint16 index = read<uint16>(cpy);
						handler(get_static{ index }); break;
					}
					case 179: {
						uint16 index = read<uint16>(cpy);
						handler(put_static{ index }); break;
					}
					case 180: {
						uint16 index = read<uint16>(cpy);
						handler(get_field{ index }); break;
					}
					case 181: {
						uint16 index = read<uint16>(cpy);
						handler(put_field{ index }); break;
					}
					case 182: {
						uint16 index = read<uint16>(cpy);
						handler(invoke_virtual{ index }); break;
					}
					case 183: {
						uint16 index = read<uint16>(cpy);
						handler(invoke_special{ index }); break;
					}
					case 184: {
						uint16 index = read<uint16>(cpy);
						handler(invoke_static{ index }); break;
					}
					case 186: {
						uint16 index = read<uint16>(cpy);
						read<uint16>(cpy); // skip two zeros
						handler(invoke_dynamic{ index }); break;
					}
					case 187: {
						uint16 index = read<uint16>(cpy);
						handler(nw{ index }); break;
					}
					case 192: {
						uint16 index = read<uint16>(cpy);
						handler(check_cast{ index }); break;
					}

					default: handler(first);
				}
			}

			return { cpy };
		}

	};

	template<typename Iterator>
	reader(Iterator) -> reader<Iterator>;

}