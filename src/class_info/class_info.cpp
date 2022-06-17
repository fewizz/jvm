#if 0

d=`realpath ${BASH_SOURCE[0]}`
d=`dirname ${d}`
root=`realpath ${d}/../../`

if clang++ \
	-std=c++20 \
	-Wall -Wextra \
	-O3 \
	-nostdinc++ \
	-I ${root}/include/ \
	-I ${root}/../core/include \
	-o ${root}/build/class_info \
	${d}/class_info.cpp
then
	${root}/build/class_info $@
fi
exit 0
#endif

#include "class/file/reader.hpp"
#include "../alloc.hpp"

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

	code_reader([&]<typename Type>(Type x, auto) {
		if constexpr (same_as<Type, nop>) fputs("nop", stdout);

		if constexpr (same_as<Type, a_const_null>) fputs("aconst_null", stdout);
		if constexpr (same_as<Type, i_const_m1>) fputs("iconst_m1", stdout);
		if constexpr (same_as<Type, i_const_0>) fputs("iconst_0", stdout);
		if constexpr (same_as<Type, i_const_1>) fputs("iconst_1", stdout);
		if constexpr (same_as<Type, i_const_2>) fputs("iconst_2", stdout);
		if constexpr (same_as<Type, i_const_3>) fputs("iconst_3", stdout);
		if constexpr (same_as<Type, i_const_4>) fputs("iconst_4", stdout);
		if constexpr (same_as<Type, i_const_5>) fputs("iconst_5", stdout);
		if constexpr (same_as<Type, l_const_0>) fputs("lconst_0", stdout);
		if constexpr (same_as<Type, l_const_1>) fputs("lconst_1", stdout);

		if constexpr (same_as<Type, bi_push>) {
			fputs("bipush ", stdout);
			printf("%hhu", x.value);
		}
		if constexpr (same_as<Type, si_push>) {
			fputs("sipush ", stdout);
			printf("%hu", x.value);
		}

		if constexpr (same_as<Type, ldc>) {
			fputs("ldc ", stdout);
			printf("%hhu", x.index);
		}
		if constexpr (same_as<Type, ldc_w>) {
			fputs("ldc_w ", stdout);
			printf("%hu", x.index);
		}

		if constexpr (same_as<Type, i_load>) {
			fputs("iload ", stdout);
			printf("%hhu", x.index);
		}
		if constexpr (same_as<Type, l_load>) {
			fputs("lload ", stdout);
			printf("%hhu", x.index);
		}
		if constexpr (same_as<Type, a_load>) {
			fputs("aload ", stdout);
			printf("%hhu", x.index);
		}
		if constexpr (same_as<Type, i_load_0>) fputs("iload_0", stdout);
		if constexpr (same_as<Type, i_load_1>) fputs("iload_1", stdout);
		if constexpr (same_as<Type, i_load_2>) fputs("iload_2", stdout);
		if constexpr (same_as<Type, i_load_3>) fputs("iload_3", stdout);
		if constexpr (same_as<Type, l_load_0>) fputs("lload_0", stdout);
		if constexpr (same_as<Type, l_load_1>) fputs("lload_1", stdout);
		if constexpr (same_as<Type, l_load_2>) fputs("lload_2", stdout);
		if constexpr (same_as<Type, l_load_3>) fputs("lload_3", stdout);
		if constexpr (same_as<Type, f_load_0>) fputs("fload_0", stdout);
		if constexpr (same_as<Type, f_load_1>) fputs("fload_1", stdout);
		if constexpr (same_as<Type, f_load_2>) fputs("fload_2", stdout);
		if constexpr (same_as<Type, f_load_3>) fputs("fload_3", stdout);
		if constexpr (same_as<Type, d_load_0>) fputs("dload_0", stdout);
		if constexpr (same_as<Type, d_load_1>) fputs("dload_1", stdout);
		if constexpr (same_as<Type, d_load_2>) fputs("dload_2", stdout);
		if constexpr (same_as<Type, d_load_3>) fputs("dload_3", stdout);
		if constexpr (same_as<Type, a_load_0>) fputs("aload_0", stdout);
		if constexpr (same_as<Type, a_load_1>) fputs("aload_1", stdout);
		if constexpr (same_as<Type, a_load_2>) fputs("aload_2", stdout);
		if constexpr (same_as<Type, a_load_3>) fputs("aload_3", stdout);
		if constexpr (same_as<Type, a_a_load>) fputs("aaload", stdout);
		if constexpr (same_as<Type, b_a_load>) fputs("baload", stdout);
		if constexpr (same_as<Type, c_a_load>) fputs("caload", stdout);

		if constexpr (same_as<Type, i_store>) {
			fputs("istore ", stdout);
			printf("%hhu", x.index);
		}
		if constexpr (same_as<Type, l_store>) {
			fputs("lstore ", stdout);
			printf("%hhu", x.index);
		}
		if constexpr (same_as<Type, a_store>) {
			fputs("astore ", stdout);
			printf("%hhu", x.index);
		}
		if constexpr (same_as<Type, i_store_0>) fputs("istore_0", stdout);
		if constexpr (same_as<Type, i_store_1>) fputs("istore_1", stdout);
		if constexpr (same_as<Type, i_store_2>) fputs("istore_2", stdout);
		if constexpr (same_as<Type, i_store_3>) fputs("istore_3", stdout);
		if constexpr (same_as<Type, a_store_0>) fputs("astore_0", stdout);
		if constexpr (same_as<Type, a_store_1>) fputs("astore_1", stdout);
		if constexpr (same_as<Type, a_store_2>) fputs("astore_2", stdout);
		if constexpr (same_as<Type, a_store_3>) fputs("astore_3", stdout);
		if constexpr (same_as<Type, a_a_store>) fputs("aastore", stdout);
		if constexpr (same_as<Type, b_a_store>) fputs("bastore", stdout);
		if constexpr (same_as<Type, c_a_store>) fputs("castore", stdout);

		if constexpr (same_as<Type, pop>) fputs("pop", stdout);
		if constexpr (same_as<Type, dup>) fputs("dup", stdout);
		if constexpr (same_as<Type, dup2>) fputs("dup2", stdout);

		if constexpr (same_as<Type, i_add>) fputs("iadd", stdout);
		if constexpr (same_as<Type, l_add>) fputs("ladd", stdout);
		if constexpr (same_as<Type, i_sub>) fputs("isub", stdout);
		if constexpr (same_as<Type, l_sub>) fputs("lsub", stdout);
		if constexpr (same_as<Type, i_mul>) fputs("imul", stdout);
		if constexpr (same_as<Type, l_mul>) fputs("lmul", stdout);
		if constexpr (same_as<Type, d_mul>) fputs("dmul", stdout);
		if constexpr (same_as<Type, i_div>) fputs("idiv", stdout);
		if constexpr (same_as<Type, i_neg>) fputs("ineg", stdout);
		if constexpr (same_as<Type, i_sh_l>) fputs("ishl", stdout);
		if constexpr (same_as<Type, l_sh_l>) fputs("lshl", stdout);
		if constexpr (same_as<Type, i_sh_r>) fputs("ishr", stdout);
		if constexpr (same_as<Type, i_and>) fputs("iand", stdout);
		if constexpr (same_as<Type, i_or>) fputs("ior", stdout);
		if constexpr (same_as<Type, i_xor>) fputs("ixor", stdout);
		if constexpr (same_as<Type, i_inc>) {
			printf("iinc ", stdout);
			printf("%hhu, %hhd", x.index, x.value);
		}
		if constexpr (same_as<Type, i_to_l>) fputs("i2l", stdout);
		if constexpr (same_as<Type, i_to_d>) fputs("i2d", stdout);
		if constexpr (same_as<Type, l_to_i>) fputs("l2i", stdout);
		if constexpr (same_as<Type, f_to_d>) fputs("f2d", stdout);
		if constexpr (same_as<Type, d_to_i>) fputs("d2i", stdout);
		if constexpr (same_as<Type, i_to_b>) fputs("i2b", stdout);
		if constexpr (same_as<Type, i_to_c>) fputs("i2c", stdout);

		if constexpr (same_as<Type, l_cmp>) fputs("lcmp", stdout);

		if constexpr (same_as<Type, if_eq>) {
			fputs("ifeq ", stdout);
			printf("%d", x.branch);
		}
		if constexpr (same_as<Type, if_ne>) {
			fputs("ifne ", stdout);
			printf("%d", x.branch);
		}
		if constexpr (same_as<Type, if_lt>) {
			fputs("iflt ", stdout);
			printf("%d", x.branch);
		}
		if constexpr (same_as<Type, if_ge>) {
			fputs("ifge ", stdout);
			printf("%d", x.branch);
		}
		if constexpr (same_as<Type, if_gt>) {
			fputs("ifgt ", stdout);
			printf("%d", x.branch);
		}
		if constexpr (same_as<Type, if_le>) {
			fputs("ifle ", stdout);
			printf("%d", x.branch);
		}
		if constexpr (same_as<Type, if_i_cmp_eq>) {
			printf("if_icmpeq %d", x.branch);
		}
		if constexpr (same_as<Type, if_i_cmp_ne>) {
			printf("if_icmpne %d", x.branch);
		}
		if constexpr (same_as<Type, if_i_cmp_lt>) {
			printf("if_icmplt %d", x.branch);
		}
		if constexpr (same_as<Type, if_i_cmp_ge>) {
			printf("if_icmpge %d", x.branch);
		}
		if constexpr (same_as<Type, if_i_cmp_gt>) {
			printf("if_icmpgt %d", x.branch);
		}
		if constexpr (same_as<Type, if_i_cmp_le>) {
			printf("if_icmple %d", x.branch);
		}
		if constexpr (same_as<Type, if_a_cmp_eq>) {
			printf("ifacmpeq %d", x.branch);
		}
		if constexpr (same_as<Type, if_a_cmp_ne>) {
			printf("ifacmpne %d", x.branch);
		}
		if constexpr (same_as<Type, go_to>) printf("goto %d", x.branch);
		if constexpr (same_as<Type, table_switch>) {
			printf("tableswitch %d", x._default);
			for(auto p : x.offsets) {
				printf(" %d", p);
			}
		}
		if constexpr (same_as<Type, lookup_switch>) {
			printf("lookupswitch %d", x._default);
			for(auto p : x.pairs) {
				printf(" %d:%d", p.match, p.offset);
			}
		}
		if constexpr (same_as<Type, i_ret>) {
			fputs("ireturn", stdout);
			return false;
		}
		if constexpr (same_as<Type, a_ret>) fputs("areturn", stdout);
		if constexpr (same_as<Type, ret>) {
			fputs("return", stdout);
			return false;
		}

		if constexpr (same_as<Type, get_static>) {
			printf("getstatic %d", x.index);
		}
		if constexpr (same_as<Type, put_static>) {
			printf("putstatic %d", x.index);
		}
		if constexpr (same_as<Type, get_field>) {
			printf("getfield %d", x.index);
		}
		if constexpr (same_as<Type, put_field>) {
			printf("putfield %d", x.index);
		}
		if constexpr (same_as<Type, invoke_virtual>) {
			printf("invokevirtual %d", x.index);
		}
		if constexpr (same_as<Type, invoke_special>) {
			printf("invokespecial %d", x.index);
		}
		if constexpr (same_as<Type, invoke_static>) {
			printf("invokestatic %hu", x.index);
		}
		if constexpr (same_as<Type, invoke_interface>) {
			printf("invokeinterface %hu %hhu", x.index, x.count);
		}
		if constexpr (same_as<Type, invoke_dynamic>) {
			printf("invokedynamic %d", x.index);
		}
		if constexpr (same_as<Type, _new>) printf("new %d", x.index);
		if constexpr (same_as<Type, new_array>) {
			printf("newarray %hhu", x.type);
		}
		if constexpr (same_as<Type, a_new_array>) {
			printf("anewarray %hu", x.index);
		}
		if constexpr (same_as<Type, array_length>) printf("arraylength");
		if constexpr (same_as<Type, a_throw>) printf("athrow");
		if constexpr (same_as<Type, check_cast>) {
			printf("checkcast %hu", x.index);
		}
		if constexpr (same_as<Type, instance_of>) {
			printf("instanceof %hu", x.index);
		}
		if constexpr (same_as<Type, monitor_enter>) {
			fputs("monitorenter", stdout);
		}
		if constexpr (same_as<Type, monitor_exit>) {
			fputs("monitorexit", stdout);
		}
		if constexpr (same_as<Type, if_null>) {
			printf("ifnull %d", x.branch);
		}
		if constexpr (same_as<Type, if_non_null>) {
			printf("ifnonnull %d", x.branch);
		}
		if constexpr (same_as<Type, uint8>) {
			printf("unknown instruction: %d\n", x);
			abort();
		}
		putchar('\n');
	});
}

int main(int argc, const char** argv) {
	if(argc != 2) {
		printf("usage: class_info.cpp <path to class file>");
		return 1;
	}

	FILE* f = fopen(argv[1], "rb");
	fseek(f, 0, SEEK_END);
	nuint size = ftell(f);
	rewind(f);
	uint8* data = (uint8*) malloc(size);
	fread(data, 1, size, f);

	using namespace class_file;

	reader read_magic{ data };
	auto [read_version, is_there_any_magic] = read_magic();

	if(!is_there_any_magic) {
		fputs("no magic here...", stderr); abort();
	}

	auto [read_constant_pool, version] = read_version();
	printf("version: %d.%d\n", version.major, version.minor);

	fputs("constant pool:\n", stdout);

	nuint entry = 0;
	auto access_flags_reader = read_constant_pool(
		[&]<typename Type>(Type x) {
			++entry;
			if constexpr (!same_as<Type, constant::skip>) {
				printf("\t[%d] ", entry);
			}

			if constexpr (same_as<Type, constant::utf8>) {
				fputs("utf8: ", stdout);
				fwrite(x.begin(), 1, x.size(), stdout);
			}
			else if constexpr (same_as<Type, constant::int32>) {
				printf("integer: value: %d", x.value);
			}
			else if constexpr (same_as<Type, constant::float32>) {
				printf("float: value: %f", x.value);
			}
			else if constexpr (same_as<Type, constant::int64>) {
				printf("long: value: %lld", x.value);
			}
			else if constexpr (same_as<Type, constant::float64>) {
				printf("double: value: %lf", x.value);
			}
			else if constexpr (same_as<Type, constant::_class>) {
				printf("class: name index: %d", x.name_index);
			}
			else if constexpr (same_as<Type, constant::string>) {
				printf("string: index: %d", x.string_index);
			}
			else if constexpr (same_as<Type, constant::field_ref>) {
				printf(
					"fieldref: class index: %d, name and type index: %d",
					x.class_index,
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, constant::method_ref>) {
				printf(
					"methodref: class index: %d, name and type index: %d",
					x.class_index,
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, constant::interface_method_ref>) {
				printf(
					"interface methodred: class index: %d",
					x.class_index
				);
				printf(
					", name and type index: %d",
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, constant::name_and_type>) {
				printf(
					"name and type: name index: %d, descriptor index: %d",
					x.name_index,
					x.descriptor_index
				);
			}
			else if constexpr (same_as<Type, constant::method_handle>) {
				printf(
					"method handle: reference kind: %d, reference index: %d",
					x.reference_kind, x.reference_index
				);
			}
			else if constexpr (same_as<Type, constant::method_type>) {
				printf(
					"method type: descriptor_index: %d",
					x.descriptor_index
				);
			}
			else if constexpr (same_as<Type, constant::dynamic>) {
				printf(
					"dynamic: bootstrap method attr index: %d",
					x.bootstrap_method_attr_index
				);

				printf(
					", name_and_type_index: %d",
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, constant::invoke_dynamic>) {
				printf(
					"invoke dynamic: bootstrap method attr index: %d",
					x.bootstrap_method_attr_index
				);

				printf(
					", name_and_type_index: %d",
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, constant::module>) {
				printf("module: name_index: %d", x.name_index);
			}
			else if constexpr (same_as<Type, constant::package>) {
				printf("package: name_index: %d", x.name_index);
			}
			else if constexpr (same_as<Type, constant::skip>) {}
			else if constexpr (same_as<Type, constant::unknown>) {
				printf("unknown tag: %d", x); abort();
			}
			else {
				printf("unimplemented tag: %d", Type::tag); abort();
			}

			printf("\n");
		}
	);

	uint16 constant_pool_size = read_constant_pool.entries_count();
	constant::utf8 utf8_strings[constant_pool_size];
	constant::_class classes[constant_pool_size];

	entry = 0;
	read_constant_pool(
		[&]<typename Type>(Type x) {
			if constexpr (same_as<Type, constant::utf8>) {
				utf8_strings[entry] = x;
			}
			if constexpr (same_as<Type, constant::_class>) {
				classes[entry] = x;
			}
			++entry;
		}
	);

	auto _class = [&](uint16 index) { return classes[index - 1]; };
	auto utf8 = [&](uint16 index) { return utf8_strings[index - 1]; };

	auto [read_this, access_flags] = access_flags_reader();
	printf("access flags: 0x%.4x\n", access_flags);

	auto [read_super, this_index] = read_this();
	printf("this class: index: %d, name: ", this_index);
	auto this_name = utf8(_class(this_index).name_index);
	fwrite(this_name.data(), 1, this_name.size(), stdout);
	printf("\n");

	auto [read_interfaces, super_index] = read_super();
	printf("super class: ");
	auto super_name = utf8(_class(super_index).name_index);
	fwrite(super_name.data(), 1, super_name.size(), stdout);
	printf("\n");

	printf("interfaces:\n");
	auto read_fields = read_interfaces([&](auto name_index) {
		printf("\t");
		auto interface_name = utf8(_class(name_index).name_index);
		fwrite(interface_name.data(), 1, interface_name.size(), stdout);
		printf("\n");
	});

	printf("fields:\n");
	auto read_methods = read_fields([&](auto field_reader) {
		auto [name_index_reader, access_flags] = field_reader();
		printf("\taccess flags: 0x%.4x, name: ", access_flags);

		auto [descriptor_index_reader, name_index] = name_index_reader();
		auto name = utf8(name_index);
		fwrite(name.data(), 1, name.size(), stdout);

		auto [read_attributes, descriptor_index] {
			descriptor_index_reader()
		};

		printf(", descriptor: ");
		auto descriptor = utf8(descriptor_index);
		fwrite(descriptor.data(), 1, descriptor.size(), stdout);
		printf("\n");

		return read_attributes(
			utf8,
			[&](auto) {

			}
		);
	});

	printf("methods:\n");
	[[maybe_unused]] auto _ = read_methods([&](auto method_reader) {
		auto [read_name_index, access_flags] = method_reader();
		printf("\taccess flags: 0x%.4x, name: ", access_flags);

		auto [read_descriptor_index, name_index] = read_name_index();
		auto name = utf8(name_index);
		fwrite(name.data(), 1, name.size(), stdout);

		auto [read_attributes, descriptor_index] = read_descriptor_index();
		auto descriptor = utf8(descriptor_index);
		printf(", descriptor: ");
		fwrite(descriptor.begin(), 1, descriptor.size(), stdout);
		printf("\n");

		return read_attributes(
			utf8,
			[&]<typename AttributeReader>(AttributeReader x) {
				if constexpr (AttributeReader::type == attribute::type::code) {
					read_code_attribute(x);
				}
			}
		);
	});

}