#pragma once

#include <core/integer.hpp>

namespace class_file::code::instruction {

	struct nop {};

	struct i_const_m1 {};
	struct i_const_0 {};
	struct i_const_1 {};
	struct i_const_2 {};
	struct i_const_3 {};
	struct i_const_4 {};
	struct i_const_5 {};

	struct ldc { uint8 index; };

	struct i_load { uint8 index; };
	struct i_load_0 {};
	struct i_load_1 {};
	struct i_load_2 {};
	struct i_load_3 {};
	struct a_load_0 {};
	struct a_load_1 {};
	struct a_load_2 {};
	struct a_load_3 {};

	struct i_store{ uint8 index; };
	struct i_store_0 {};
	struct i_store_1 {};
	struct i_store_2 {};
	struct i_store_3 {};
	struct a_store_0 {};
	struct a_store_1 {};
	struct a_store_2 {};
	struct a_store_3 {};

	struct pop {};
	struct dup {};

	struct if_eq { uint16 branch; };
	struct if_ne { uint16 branch; };
	struct if_lt { uint16 branch; };
	struct if_ge { uint16 branch; };
	struct if_gt { uint16 branch; };
	struct if_le { uint16 branch; };
	struct if_a_cmp_eq { uint16 branch; };
	struct if_a_cmp_ne { uint16 branch; };
	struct go_to { uint16 branch; };

	struct rtrn {};

	struct get_static { uint16 index; };
	struct put_static { uint16 index; };
	struct get_field { uint16 index; };
	struct put_field { uint16 index; };
	struct invoke_virtual { uint16 index; };
	struct invoke_special { uint16 index; };
	struct invoke_static { uint16 index; };
	struct invoke_dynamic { uint16 index; };

	struct nw { uint16 index; };

	struct check_cast{ uint16 index; };

}