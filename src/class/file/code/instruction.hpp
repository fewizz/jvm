#pragma once

#include <core/span.hpp>

namespace class_file::code::instruction {

	struct nop {};

	struct a_const_null{};
	struct i_const_m1 {};
	struct i_const_0 {};
	struct i_const_1 {};
	struct i_const_2 {};
	struct i_const_3 {};
	struct i_const_4 {};
	struct i_const_5 {};
	struct l_const_0 {};
	struct l_const_1 {};

	struct bi_push { uint8 value; };
	struct si_push { uint16 value; };

	struct ldc { uint8 index; };
	struct ldc_w { uint16 index; };

	struct i_load { uint8 index; };
	struct l_load { uint8 index; };
	struct a_load { uint8 index; };
	struct i_load_0 {};
	struct i_load_1 {};
	struct i_load_2 {};
	struct i_load_3 {};
	struct l_load_0 {};
	struct l_load_1 {};
	struct l_load_2 {};
	struct l_load_3 {};
	struct f_load_0 {};
	struct f_load_1 {};
	struct f_load_2 {};
	struct f_load_3 {};
	struct d_load_0 {};
	struct d_load_1 {};
	struct d_load_2 {};
	struct d_load_3 {};
	struct a_load_0 {};
	struct a_load_1 {};
	struct a_load_2 {};
	struct a_load_3 {};
	struct aa_load {};
	struct ba_load {};
	struct ca_load {};

	struct i_store{ uint8 index; };
	struct l_store{ uint8 index; };
	struct a_store{ uint8 index; };
	struct i_store_0 {};
	struct i_store_1 {};
	struct i_store_2 {};
	struct i_store_3 {};
	struct a_store_0 {};
	struct a_store_1 {};
	struct a_store_2 {};
	struct a_store_3 {};
	struct aa_store {};
	struct ba_store {};
	struct ca_store {};

	struct pop {};
	struct dup {};
	struct dup2 {};

	struct i_add {};
	struct l_add {};
	struct i_sub {};
	struct l_sub {};
	struct i_mul {};
	struct l_mul {};
	struct d_mul {};
	struct i_div {};
	struct i_neg {};
	struct i_sh_l {};
	struct l_sh_l {};
	struct i_sh_r {};
	struct i_and {};
	struct i_or {};
	struct i_xor {};
	struct i_inc { uint8 index; int8 value; };
	struct i_to_l {};
	struct i_to_d {};
	struct l_to_i {};
	struct f_to_d {};
	struct d_to_i {};
	struct i_to_b {};
	struct i_to_c {};

	struct l_cmp {};

	struct if_eq { uint16 branch; };
	struct if_ne { uint16 branch; };
	struct if_lt { uint16 branch; };
	struct if_ge { uint16 branch; };
	struct if_gt { uint16 branch; };
	struct if_le { uint16 branch; };
	struct if_i_cmp_eq { uint16 branch; };
	struct if_i_cmp_ne { uint16 branch; };
	struct if_i_cmp_lt { uint16 branch; };
	struct if_i_cmp_ge { uint16 branch; };
	struct if_i_cmp_gt { uint16 branch; };
	struct if_i_cmp_le { uint16 branch; };
	struct if_a_cmp_eq { uint16 branch; };
	struct if_a_cmp_ne { uint16 branch; };
	struct go_to { uint16 branch; };

	struct match_offset{ int32 match; int32 offset; };
	struct lookup_switch { int32 _default; span<match_offset, uint32> pairs; };

	struct i_return {};
	struct a_return {};
	struct _return {};

	struct get_static { uint16 index; };
	struct put_static { uint16 index; };
	struct get_field { uint16 index; };
	struct put_field { uint16 index; };
	struct invoke_virtual { uint16 index; };
	struct invoke_special { uint16 index; };
	struct invoke_static { uint16 index; };
	struct invoke_interface { uint16 index; uint8 count; };
	struct invoke_dynamic { uint16 index; };

	struct _new { uint16 index; };
	struct new_array { uint8 type; };
	struct a_new_array { uint16 index; };
	struct array_length {};

	struct a_throw {};

	struct check_cast { uint16 index; };
	struct instance_of { uint16 index; };
	struct monitor_enter {};
	struct monitor_exit {};

	struct if_null { uint16 branch; };
	struct if_non_null { uint16 branch; };

}