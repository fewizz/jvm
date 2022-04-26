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
			uint16 max_stack = read<uint16, endianness::big>(cpy);
			return { { cpy }, { max_stack } };
		}

		elements::of<reader<Iterator, reader_stage::code>, uint16>
		operator () () const
		requires (Stage == reader_stage::max_locals) {
			Iterator cpy = src;
			uint16 max_locals = read<uint16, endianness::big>(cpy);
			return { { cpy }, { max_locals } };
		}

		template<typename Handler>
		reader<Iterator, reader_stage::exception_table>
		operator () (Handler&& handler) const
		requires (Stage == reader_stage::code) {
			using namespace class_file::code::instruction;

			Iterator cpy = src;
			uint32 length = read<uint32, endianness::big>(cpy);

			auto src0 = cpy;

			while(cpy - src0 < length) {
				uint8 first = read<uint8>(cpy);

				switch (first) {
					case 0: handler(nop{}); break;

					case 1: handler(a_const_null{}); break;
					case 2: handler(i_const_m1{}); break;
					case 3: handler(i_const_0{}); break;
					case 4: handler(i_const_1{}); break;
					case 5: handler(i_const_2{}); break;
					case 6: handler(i_const_3{}); break;
					case 7: handler(i_const_4{}); break;
					case 8: handler(i_const_5{}); break;
					case 9: handler(l_const_0{}); break;
					case 10: handler(l_const_1{}); break;

					case 16: {
						uint8 value = read<uint8>(cpy);
						handler(bi_push{ value }); break;
					}
					case 17: {
						uint16 value = read<uint16, endianness::big>(cpy);
						handler(si_push{ value }); break;
					}

					case 18: {
						uint8 index = read<uint8>(cpy);
						handler(ldc{ index }); break;
					}
					case 19: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(ldc_w{ index }); break;
					}
					case 21: {
						uint8 index = read<uint8>(cpy);
						handler(i_load{ index }); break;
					}
					case 22: {
						uint8 index = read<uint8>(cpy);
						handler(l_load{ index }); break;
					}
					case 25: {
						uint8 index = read<uint8>(cpy);
						handler(a_load{ index }); break;
					}
					case 26: handler(i_load_0{}); break;
					case 27: handler(i_load_1{}); break;
					case 28: handler(i_load_2{}); break;
					case 29: handler(i_load_3{}); break;
					case 30: handler(l_load_0{}); break;
					case 31: handler(l_load_1{}); break;
					case 32: handler(l_load_2{}); break;
					case 33: handler(l_load_3{}); break;
					case 34: handler(f_load_0{}); break;
					case 35: handler(f_load_1{}); break;
					case 36: handler(f_load_2{}); break;
					case 37: handler(f_load_3{}); break;
					case 38: handler(d_load_0{}); break;
					case 39: handler(d_load_1{}); break;
					case 40: handler(d_load_2{}); break;
					case 41: handler(d_load_3{}); break;
					case 42: handler(a_load_0{}); break;
					case 43: handler(a_load_1{}); break;
					case 44: handler(a_load_2{}); break;
					case 45: handler(a_load_3{}); break;

					case 50: handler(aa_load{}); break;
					case 51: handler(ba_load{}); break;
					case 52: handler(ca_load{}); break;
					case 54: {
						uint8 index = read<uint8>(cpy);
						handler(i_store{ index }); break;
					}
					case 55: {
						uint8 index = read<uint8>(cpy);
						handler(l_store{ index }); break;
					}
					case 58: {
						uint8 index = read<uint8>(cpy);
						handler(a_store{ index }); break;
					}
					case 59: handler(i_store_0{}); break;
					case 60: handler(i_store_1{}); break;
					case 61: handler(i_store_2{}); break;
					case 62: handler(i_store_3{}); break;
					case 75: handler(a_store_0{}); break;
					case 76: handler(a_store_1{}); break;
					case 77: handler(a_store_2{}); break;
					case 78: handler(a_store_3{}); break;
					case 83: handler(aa_store{}); break;
					case 84: handler(ba_store{}); break;
					case 85: handler(ca_store{}); break;
					case 87: handler(pop{}); break;
					case 89: handler(dup{}); break;
					case 92: handler(dup2{}); break;

					case 96: handler(i_add{}); break;
					case 97: handler(l_add{}); break;
					case 100: handler(i_sub{}); break;
					case 101: handler(l_sub{}); break;
					case 104: handler(i_mul{}); break;
					case 105: handler(l_mul{}); break;
					case 107: handler(d_mul{}); break;
					case 108: handler(i_div{}); break;
					case 116: handler(i_neg{}); break;
					case 120: handler(i_sh_l{}); break;
					case 121: handler(l_sh_l{}); break;
					case 122: handler(i_sh_r{}); break;
					case 126: handler(i_and{}); break;
					case 128: handler(i_or{}); break;
					case 130: handler(i_xor{}); break;
					case 132: {
						uint8 index = read<uint8>(cpy);
						int8 value = read<int8>(cpy);
						handler(i_inc{ index, value }); break;
					}
					case 133: handler(i_to_l{}); break;
					case 135: handler(i_to_d{}); break;
					case 136: handler(l_to_i{}); break;
					case 141: handler(f_to_d{}); break;
					case 142: handler(d_to_i{}); break;
					case 145: handler(i_to_b{}); break;
					case 146: handler(i_to_c{}); break;

					case 148: handler(l_cmp{}); break;

					case 153: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_eq{ branch }); break;
					}
					case 154: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_ne{ branch }); break;
					}
					case 155: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_lt{ branch }); break;
					}
					case 156: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_ge{ branch }); break;
					}
					case 157: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_gt{ branch }); break;
					}
					case 158: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_le{ branch }); break;
					}
					case 159: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_i_cmp_eq{ branch }); break;
					}
					case 160: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_i_cmp_ne{ branch }); break;
					}
					case 161: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_i_cmp_lt{ branch }); break;
					}
					case 162: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_i_cmp_ge{ branch }); break;
					}
					case 163: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_i_cmp_gt{ branch }); break;
					}
					case 164: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_i_cmp_le{ branch }); break;
					}
					case 165: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_a_cmp_eq{ branch }); break;
					}
					case 166: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_a_cmp_ne{ branch }); break;
					}
					case 167: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(go_to{ branch }); break;
					}
					case 171: {
						while((cpy - src0) % 4 != 0) {
							++cpy;
						}
						int32 _default = read<int32, endianness::big>(cpy);
						uint32 n_pairs = read<uint32, endianness::big>(cpy);
						match_offset storage[n_pairs];
						for(nuint z = 0; z < n_pairs; ++z) {
							int32 match = read<int32, endianness::big>(cpy);
							int32 offset = read<int32, endianness::big>(cpy);
							storage[z] = match_offset{ match, offset };
						}
						span<match_offset, uint32> pairs{ storage, n_pairs };
						handler(lookup_switch{ _default, pairs }); break;
					}
					case 172: handler(i_return{}); break;
					case 176: handler(a_return{}); break;
					case 177: handler(_return{}); break;

					case 178: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(get_static{ index }); break;
					}
					case 179: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(put_static{ index }); break;
					}
					case 180: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(get_field{ index }); break;
					}
					case 181: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(put_field{ index }); break;
					}
					case 182: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(invoke_virtual{ index }); break;
					}
					case 183: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(invoke_special{ index }); break;
					}
					case 184: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(invoke_static{ index }); break;
					}
					case 185: {
						uint16 index = read<uint16, endianness::big>(cpy);
						uint8 count = read<uint8>(cpy);
						read<uint8>(cpy); // skip 0
						handler(invoke_interface{ index, count }); break;
					}
					case 186: {
						uint16 index = read<uint16, endianness::big>(cpy);
						read<uint16>(cpy); // skip two zeros
						handler(invoke_dynamic{ index }); break;
					}
					case 187: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(_new{ index }); break;
					}
					case 188: {
						uint8 type = read<uint8>(cpy);
						handler(new_array{ type }); break;
					}
					case 189: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(a_new_array{ index }); break;
					}
					case 190: handler(array_length{}); break;
					case 191: handler(a_throw{}); break;
					case 192: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(check_cast{ index }); break;
					}
					case 193: {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(instance_of{ index }); break;
					}
					case 194: handler(monitor_enter{}); break;
					case 195: handler(monitor_exit{}); break;
					case 198: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_null{ branch }); break;
					}
					case 199: {
						uint16 branch = read<uint16, endianness::big>(cpy);
						handler(if_non_null{ branch }); break;
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