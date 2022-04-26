#if 0

d=`realpath ${BASH_SOURCE[0]}`
d=`dirname ${d}`

clang++ \
	-std=c++20 \
	-Wall -Wextra \
	-g \
	-O0 \
	-Wl,--stack,0x1000000 \
	-nostdinc++ \
	-I ${d}/../src \
	-I ${d}/../../core/include \
	-o ${d}/class_info \
	${d}/class_info.cpp

${d}/class_info

exit 0

#endif

#include "class/file/reader.hpp"
#include "alloc.hpp"

#include <stdio.h>

extern "C" void abort();

template<typename Iterator>
void read_code_attribute(
	class_file::attribute::code::reader<Iterator> max_stack_reader
) {
	auto [max_locals_reader, max_stack] = max_stack_reader();
	printf("max stack: %d\n", max_stack);

	auto [code_reader, max_locals] = max_locals_reader();
	printf("max locals: %d\n", max_locals);

	using namespace class_file::code::instruction;

	code_reader([&]<typename Type>(Type x) {
		if constexpr (same_as<Type, nop>) printf("nop\n");

		if constexpr (same_as<Type, a_const_null>) printf("aconst_null\n");
		if constexpr (same_as<Type, i_const_m1>) printf("iconst_m1\n");
		if constexpr (same_as<Type, i_const_0>) printf("iconst_0\n");
		if constexpr (same_as<Type, i_const_1>) printf("iconst_1\n");
		if constexpr (same_as<Type, i_const_2>) printf("iconst_2\n");
		if constexpr (same_as<Type, i_const_3>) printf("iconst_3\n");
		if constexpr (same_as<Type, i_const_4>) printf("iconst_4\n");
		if constexpr (same_as<Type, i_const_5>) printf("iconst_5\n");
		if constexpr (same_as<Type, l_const_0>) printf("lconst_0\n");
		if constexpr (same_as<Type, l_const_1>) printf("lconst_1\n");

		if constexpr (same_as<Type, bi_push>) printf("bipush %hhu\n", x.value);
		if constexpr (same_as<Type, si_push>) printf("sipush %hu\n", x.value);

		if constexpr (same_as<Type, ldc>) printf("ldc %hhu\n", x.index);
		if constexpr (same_as<Type, ldc_w>) printf("ldc_w %hu\n", x.index);

		if constexpr (same_as<Type, i_load>) printf("iload %hhu\n", x.index);
		if constexpr (same_as<Type, l_load>) printf("lload %hhu\n", x.index);
		if constexpr (same_as<Type, a_load>) printf("aload %hhu\n", x.index);
		if constexpr (same_as<Type, i_load_0>) printf("iload_0\n");
		if constexpr (same_as<Type, i_load_1>) printf("iload_1\n");
		if constexpr (same_as<Type, i_load_2>) printf("iload_2\n");
		if constexpr (same_as<Type, i_load_3>) printf("iload_3\n");
		if constexpr (same_as<Type, l_load_0>) printf("lload_0\n");
		if constexpr (same_as<Type, l_load_1>) printf("lload_1\n");
		if constexpr (same_as<Type, l_load_2>) printf("lload_2\n");
		if constexpr (same_as<Type, l_load_3>) printf("lload_3\n");
		if constexpr (same_as<Type, f_load_0>) printf("fload_0\n");
		if constexpr (same_as<Type, f_load_1>) printf("fload_1\n");
		if constexpr (same_as<Type, f_load_2>) printf("fload_2\n");
		if constexpr (same_as<Type, f_load_3>) printf("fload_3\n");
		if constexpr (same_as<Type, d_load_0>) printf("dload_0\n");
		if constexpr (same_as<Type, d_load_1>) printf("dload_1\n");
		if constexpr (same_as<Type, d_load_2>) printf("dload_2\n");
		if constexpr (same_as<Type, d_load_3>) printf("dload_3\n");
		if constexpr (same_as<Type, a_load_0>) printf("aload_0\n");
		if constexpr (same_as<Type, a_load_1>) printf("aload_1\n");
		if constexpr (same_as<Type, a_load_2>) printf("aload_2\n");
		if constexpr (same_as<Type, a_load_3>) printf("aload_3\n");
		if constexpr (same_as<Type, aa_load>) printf("aaload\n");
		if constexpr (same_as<Type, ba_load>) printf("baload\n");
		if constexpr (same_as<Type, ca_load>) printf("caload\n");

		if constexpr (same_as<Type, i_store>) printf("istore %hhu\n", x.index);
		if constexpr (same_as<Type, l_store>) printf("lstore %hhu\n", x.index);
		if constexpr (same_as<Type, a_store>) printf("astore %hhu\n", x.index);
		if constexpr (same_as<Type, i_store_0>) printf("istore_0\n");
		if constexpr (same_as<Type, i_store_1>) printf("istore_1\n");
		if constexpr (same_as<Type, i_store_2>) printf("istore_2\n");
		if constexpr (same_as<Type, i_store_3>) printf("istore_3\n");
		if constexpr (same_as<Type, a_store_0>) printf("astore_0\n");
		if constexpr (same_as<Type, a_store_1>) printf("astore_1\n");
		if constexpr (same_as<Type, a_store_2>) printf("astore_2\n");
		if constexpr (same_as<Type, a_store_3>) printf("astore_3\n");
		if constexpr (same_as<Type, aa_store>) printf("aastore\n");
		if constexpr (same_as<Type, ba_store>) printf("bastore\n");
		if constexpr (same_as<Type, ca_store>) printf("castore\n");

		if constexpr (same_as<Type, pop>) printf("pop\n");
		if constexpr (same_as<Type, dup>) printf("dup\n");
		if constexpr (same_as<Type, dup2>) printf("dup2\n");

		if constexpr (same_as<Type, i_add>) printf("iadd\n");
		if constexpr (same_as<Type, l_add>) printf("ladd\n");
		if constexpr (same_as<Type, i_sub>) printf("isub\n");
		if constexpr (same_as<Type, l_sub>) printf("lsub\n");
		if constexpr (same_as<Type, i_mul>) printf("imul\n");
		if constexpr (same_as<Type, l_mul>) printf("lmul\n");
		if constexpr (same_as<Type, d_mul>) printf("dmul\n");
		if constexpr (same_as<Type, i_div>) printf("idiv\n");
		if constexpr (same_as<Type, i_neg>) printf("ineg\n");
		if constexpr (same_as<Type, i_sh_l>) printf("ishl\n");
		if constexpr (same_as<Type, l_sh_l>) printf("lshl\n");
		if constexpr (same_as<Type, i_sh_r>) printf("ishr\n");
		if constexpr (same_as<Type, i_and>) printf("iand\n");
		if constexpr (same_as<Type, i_or>) printf("ior\n");
		if constexpr (same_as<Type, i_xor>) printf("ixor\n");
		if constexpr (same_as<Type, i_inc>) {
			printf("iinc %hhu, %hhd\n", x.index, x.value);
		}
		if constexpr (same_as<Type, i_to_l>) printf("i2l\n");
		if constexpr (same_as<Type, i_to_d>) printf("i2d\n");
		if constexpr (same_as<Type, l_to_i>) printf("l2i\n");
		if constexpr (same_as<Type, f_to_d>) printf("f2d\n");
		if constexpr (same_as<Type, d_to_i>) printf("d2i\n");
		if constexpr (same_as<Type, i_to_b>) printf("i2b\n");
		if constexpr (same_as<Type, i_to_c>) printf("i2c\n");

		if constexpr (same_as<Type, l_cmp>) printf("lcmp\n");

		if constexpr (same_as<Type, if_eq>) printf("ifeq %d\n", x.branch);
		if constexpr (same_as<Type, if_ne>) printf("ifne %d\n", x.branch);
		if constexpr (same_as<Type, if_lt>) printf("iflt %d\n", x.branch);
		if constexpr (same_as<Type, if_ge>) printf("ifge %d\n", x.branch);
		if constexpr (same_as<Type, if_gt>) printf("ifgt %d\n", x.branch);
		if constexpr (same_as<Type, if_le>) printf("ifle %d\n", x.branch);
		if constexpr (same_as<Type, if_i_cmp_eq>) {
			printf("if_icmpeq %d\n", x.branch);
		}
		if constexpr (same_as<Type, if_i_cmp_ne>) {
			printf("if_icmpne %d\n", x.branch);
		}
		if constexpr (same_as<Type, if_i_cmp_lt>) {
			printf("if_icmplt %d\n", x.branch);
		}
		if constexpr (same_as<Type, if_i_cmp_ge>) {
			printf("if_icmpge %d\n", x.branch);
		}
		if constexpr (same_as<Type, if_i_cmp_gt>) {
			printf("if_icmpgt %d\n", x.branch);
		}
		if constexpr (same_as<Type, if_i_cmp_le>) {
			printf("if_icmple %d\n", x.branch);
		}
		if constexpr (same_as<Type, if_a_cmp_eq>) {
			printf("ifacmpeq %d\n", x.branch);
		}
		if constexpr (same_as<Type, if_a_cmp_ne>) {
			printf("ifacmpne %d\n", x.branch);
		}
		if constexpr (same_as<Type, go_to>) printf("goto %d\n", x.branch);
		if constexpr (same_as<Type, lookup_switch>) {
			printf("lookupswitch %d", x._default);
			for(auto p : x.pairs) {
				printf(" %d:%d", p.match, p.offset);
			}
			printf("\n");
		}
		if constexpr (same_as<Type, i_return>) printf("ireturn\n");
		if constexpr (same_as<Type, a_return>) printf("areturn\n");
		if constexpr (same_as<Type, _return>) printf("return\n");

		if constexpr (same_as<Type, get_static>) {
			printf("getstatic %d\n", x.index);
		}
		if constexpr (same_as<Type, put_static>) {
			printf("putstatic %d\n", x.index);
		}
		if constexpr (same_as<Type, get_field>) {
			printf("getfield %d\n", x.index);
		}
		if constexpr (same_as<Type, put_field>) {
			printf("putfield %d\n", x.index);
		}
		if constexpr (same_as<Type, invoke_virtual>) {
			printf("invokevirtual %d\n", x.index);
		}
		if constexpr (same_as<Type, invoke_special>) {
			printf("invokespecial %d\n", x.index);
		}
		if constexpr (same_as<Type, invoke_static>) {
			printf("invokestatic %hu\n", x.index);
		}
		if constexpr (same_as<Type, invoke_interface>) {
			printf("invokeinterface %hu %hhu\n", x.index, x.count);
		}
		if constexpr (same_as<Type, invoke_dynamic>) {
			printf("invokedynamic %d\n", x.index);
		}
		if constexpr (same_as<Type, _new>) printf("new %d\n", x.index);
		if constexpr (same_as<Type, new_array>) {
			printf("newarray %hhu\n", x.type);
		}
		if constexpr (same_as<Type, a_new_array>) {
			printf("anewarray %hu\n", x.index);
		}
		if constexpr (same_as<Type, array_length>) printf("arraylength\n");
		if constexpr (same_as<Type, a_throw>) printf("athrow\n");
		if constexpr (same_as<Type, check_cast>) {
			printf("checkcast %hu\n", x.index);
		}
		if constexpr (same_as<Type, instance_of>) {
			printf("instanceof %hu\n", x.index);
		}
		if constexpr (same_as<Type, monitor_enter>) printf("monitorenter\n");
		if constexpr (same_as<Type, monitor_exit>) printf("monitorexit\n");
		if constexpr (same_as<Type, if_null>) {
			printf("ifnull %d\n", x.branch);
		}
		if constexpr (same_as<Type, if_non_null>) {
			printf("ifnonnull %d\n", x.branch);
		}
		if constexpr (same_as<Type, uint8>) {
			printf("unknown instruction: %d\n", x);
			abort();
		}
	});
}

int main() {
	FILE* f = fopen("java.base/java/lang/String.class", "rb");

	fseek(f, 0, SEEK_END);
	nuint size = ftell(f);
	rewind(f);

	uint8* data = (uint8*) malloc(size);

	fread(data, 1, size, f);

	using namespace class_file;

	reader magic_reader{ data };
	auto [version_reader, is_here_any_magic] = magic_reader();

	if(!is_here_any_magic) {
		printf("no magic here...");
		abort();
	}

	auto [constant_pool_reader, version] = version_reader();

	printf("version: %d.%d\n", version.major, version.minor);

	printf("constant pool:\n");

	nuint constant_pool_entry_index = 1;

	auto access_flags_reader = constant_pool_reader(
		[&]<typename Type>(Type x) {
			if constexpr (!same_as<Type, constant::skip>) {
				printf("\t[%d] ", constant_pool_entry_index);
			}

			++constant_pool_entry_index;

			if constexpr (same_as<Type, constant::utf8>) {
				printf("utf8: ");
				fwrite(x.begin(), 1, x.size(), stdout);
				printf("\n");
			}
			else if constexpr (same_as<Type, constant::integer>) {
				printf("integer: value: %d\n", x.value);
			}
			else if constexpr (same_as<Type, constant::_float>) {
				printf("float: value: %f\n", x.value);
			}
			else if constexpr (same_as<Type, constant::_long>) {
				printf("long: value: %lld\n", x.value);
			}
			else if constexpr (same_as<Type, constant::_double>) {
				printf("double: value: %lf\n", x.value);
			}
			else if constexpr (same_as<Type, constant::_class>) {
				printf("class: name index: %d\n", x.name_index);
			}
			else if constexpr (same_as<Type, constant::string>) {
				printf("string: index: %d\n", x.string_index);
			}
			else if constexpr (same_as<Type, constant::fieldref>) {
				printf(
					"fieldref: class index: %d, name and type index: %d\n",
					x.class_index,
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, constant::methodref>) {
				printf(
					"methodref: class index: %d, name and type index: %d\n",
					x.class_index,
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, constant::interface_methodref>) {
				printf(
					"interface methodred: class index: %d",
					x.class_index
				);
				printf(
					", name and type index: %d\n",
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, constant::name_and_type>) {
				printf(
					"name and type: name index: %d, descriptor index: %d\n",
					x.name_index,
					x.descriptor_index
				);
			}
			else if constexpr (same_as<Type, constant::method_handle>) {
				printf(
					"method handle: reference kind: %d, reference index: %d\n",
					x.reference_kind, x.reference_index
				);
			}
			else if constexpr (same_as<Type, constant::method_type>) {
				printf(
					"method type: descriptor_index: %d\n",
					x.descriptor_index
				);
			}
			else if constexpr (same_as<Type, constant::dynamic>) {
				printf(
					"dynamic: bootstrap method attr index: %d",
					x.bootstrap_method_attr_index
				);

				printf(
					", name_and_type_index: %d\n",
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, constant::invoke_dynamic>) {
				printf(
					"invoke dynamic: bootstrap method attr index: %d",
					x.bootstrap_method_attr_index
				);

				printf(
					", name_and_type_index: %d\n",
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, constant::module>) {
				printf("module: name_index: %d\n", x.name_index);
			}
			else if constexpr (same_as<Type, constant::package>) {
				printf("package: name_index: %d\n", x.name_index);
			}
			else if constexpr (same_as<Type, constant::skip>) {}
			else if constexpr (same_as<Type, uint8>) {
				printf("unknown tag: %d", x);
				abort();
			}
			else {
				x++;
			}
		}
	);

	constant::utf8 utf8_strings[constant_pool_entry_index];
	constant::_class classes[constant_pool_entry_index];

	constant_pool_entry_index = 1;

	constant_pool_reader(
		[&]<typename Type>(Type x) {
			if constexpr (same_as<Type, constant::utf8>) {
				utf8_strings[constant_pool_entry_index] = x;
			}
			if constexpr (same_as<Type, constant::_class>) {
				classes[constant_pool_entry_index] = x;
			}
			++constant_pool_entry_index;
		}
	);

	auto [this_reader, access_flags] = access_flags_reader();
	printf("access flags: 0x%.4x\n", access_flags);

	auto [super_reader, this_class] = this_reader();
	printf("this class: index: %d, name: ", this_class);
	auto this_name = utf8_strings[classes[this_class].name_index];
	fwrite(this_name.begin(), 1, this_name.size(), stdout);
	printf("\n");

	auto [interfaces_reader, super_class] = super_reader();
	printf("super class: ");
	fwrite(
		utf8_strings[classes[super_class].name_index].begin(),
		1,
		utf8_strings[classes[super_class].name_index].size(),
		stdout
	);
	printf("\n");

	printf("interfaces:\n");
	auto fields_reader = interfaces_reader([&](auto index) {
		printf("\t");
		auto name = utf8_strings[classes[index].name_index];
		fwrite(name.begin(), 1, name.size(), stdout);
		printf("\n");
	});

	printf("fields:\n");
	auto methods_reader = fields_reader(
		[&](auto field_reader) {
			auto [name_index_reader, access_flags] = field_reader();
			printf("\taccess flags: 0x%.4x, name: ", access_flags);

			auto [descriptor_index_reader, name_index] = name_index_reader();

			fwrite(
				utf8_strings[name_index].begin(),
				1,
				utf8_strings[name_index].size(),
				stdout
			);

			auto [attributes_reader, descriptor_index] {
				descriptor_index_reader()
			};

			printf(", descriptor: ");
			fwrite(
				utf8_strings[descriptor_index].begin(),
				1,
				utf8_strings[descriptor_index].size(),
				stdout
			);
			printf("\n");

			return attributes_reader(
				[&]<typename Iterator>(attribute::reader<Iterator> attribute_reader) {
					return attribute_reader(
						[&](auto index){ return utf8_strings[index]; },
						[&]<typename Reader>(Reader reader) {
						}
					);
				}
			);
		}
	);

	printf("methods:\n");
	auto _ = methods_reader(
		[&](auto method_reader) {
			auto [name_index_reader, access_flags] = method_reader();
			printf("\taccess flags: 0x%.4x, name: ", access_flags);

			auto [descriptor_index_reader, name_index] = name_index_reader();

			fwrite(
				utf8_strings[name_index].begin(),
				1,
				utf8_strings[name_index].size(),
				stdout
			);

			auto [attributes_reader, descriptor_index] {
				descriptor_index_reader()
			};

			printf(", descriptor: ");
			fwrite(
				utf8_strings[descriptor_index].begin(),
				1,
				utf8_strings[descriptor_index].size(),
				stdout
			);
			printf("\n");

			return attributes_reader(
				[&]<typename Iterator>(attribute::reader<Iterator> attribute_reader) {
					return attribute_reader(
						[&](auto index){ return utf8_strings[index]; },
						[&]<typename Reader>(Reader reader) {
							if constexpr (
								same_as<
									Reader, attribute::code::reader<Iterator>
								>
							) {
								read_code_attribute(reader);
							}
						}
					);
				}
			);
		}
	);

}