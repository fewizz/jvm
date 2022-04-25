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
#include "class/file/code/reader.hpp"
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

		if constexpr (same_as<Type, i_const_m1>) printf("iconst_m1\n");
		if constexpr (same_as<Type, i_const_0>) printf("iconst_0\n");
		if constexpr (same_as<Type, i_const_1>) printf("iconst_1\n");
		if constexpr (same_as<Type, i_const_2>) printf("iconst_2\n");
		if constexpr (same_as<Type, i_const_3>) printf("iconst_3\n");
		if constexpr (same_as<Type, i_const_4>) printf("iconst_4\n");
		if constexpr (same_as<Type, i_const_5>) printf("iconst_5\n");

		if constexpr (same_as<Type, ldc>) printf("ldc %d\n", x.index);
		if constexpr (same_as<Type, i_load>) printf("iload %d\n", x.index);
		if constexpr (same_as<Type, i_load_0>) printf("iload 0\n");
		if constexpr (same_as<Type, i_load_1>) printf("iload 1\n");
		if constexpr (same_as<Type, i_load_2>) printf("iload 2\n");
		if constexpr (same_as<Type, i_load_3>) printf("iload 3\n");
		if constexpr (same_as<Type, a_load_0>) printf("aload 0\n");
		if constexpr (same_as<Type, a_load_1>) printf("aload 1\n");
		if constexpr (same_as<Type, a_load_2>) printf("aload 2\n");
		if constexpr (same_as<Type, a_load_3>) printf("aload 3\n");

		if constexpr (same_as<Type, i_store>) printf("istore %d\n", x.index);
		if constexpr (same_as<Type, i_store_0>) printf("istore 0\n");
		if constexpr (same_as<Type, i_store_1>) printf("istore 1\n");
		if constexpr (same_as<Type, i_store_2>) printf("istore 2\n");
		if constexpr (same_as<Type, i_store_3>) printf("istore 3\n");
		if constexpr (same_as<Type, a_store_0>) printf("astore 0\n");
		if constexpr (same_as<Type, a_store_1>) printf("astore 1\n");
		if constexpr (same_as<Type, a_store_2>) printf("astore 2\n");
		if constexpr (same_as<Type, a_store_3>) printf("astore 3\n");

		if constexpr (same_as<Type, pop>) printf("pop\n");
		if constexpr (same_as<Type, dup>) printf("dup\n");

		if constexpr (same_as<Type, if_eq>) printf("ifeq %d\n", x.branch);
		if constexpr (same_as<Type, if_ne>) printf("ifne %d\n", x.branch);
		if constexpr (same_as<Type, if_lt>) printf("iflt %d\n", x.branch);
		if constexpr (same_as<Type, if_ge>) printf("ifge %d\n", x.branch);
		if constexpr (same_as<Type, if_gt>) printf("ifgt %d\n", x.branch);
		if constexpr (same_as<Type, if_le>) printf("ifle %d\n", x.branch);
		if constexpr (same_as<Type, if_a_cmp_eq>) {
			printf("ifacmpeq %d\n", x.branch);
		}
		if constexpr (same_as<Type, if_a_cmp_ne>) {
			printf("ifacmpne %d\n", x.branch);
		}
		if constexpr (same_as<Type, go_to>) printf("goto %d\n", x.branch);

		if constexpr (same_as<Type, rtrn>) printf("return\n");

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
			printf("invokestatic %d\n", x.index);
		}
		if constexpr (same_as<Type, invoke_dynamic>) {
			printf("invokedynamic %d\n", x.index);
		}
		if constexpr (same_as<Type, nw>) printf("new %d\n", x.index);
		if constexpr (same_as<Type, check_cast>) {
			printf("checkcast %d\n", x.index);
		}
		if constexpr (same_as<Type, uint8>) {
			printf("unknown instruction: %d\n", x);
			abort();
		}
	});
}

int main() {
	FILE* f = fopen("PlayerEntityMixin.class", "rb");

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
			printf("\t[%d] ", constant_pool_entry_index++);

			if constexpr (same_as<Type, constant::utf8>) {
				printf("utf8: ");
				fwrite(x.begin(), 1, x.size(), stdout);
				printf("\n");
			}
			else if constexpr (same_as<Type, constant::integer>) {
				printf("integer: value: %d\n", x.value);
			}
			else if constexpr (same_as<Type, constant::floating>) {
				printf("float: value: %d\n", x.value);
			}
			else if constexpr (same_as<Type, constant::long_integer>) {
				printf("long: value: %d\n", x.value);
			}
			else if constexpr (same_as<Type, constant::double_floating>) {
				printf("double: value: %d\n", x.value);
			}
			else if constexpr (same_as<Type, constant::clazz>) {
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
	constant::clazz classes[constant_pool_entry_index];

	constant_pool_entry_index = 1;

	constant_pool_reader(
		[&]<typename Type>(Type x) {
			if constexpr (same_as<Type, constant::utf8>) {
				utf8_strings[constant_pool_entry_index] = x;
			}
			if constexpr (same_as<Type, constant::clazz>) {
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