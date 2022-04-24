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

int main() {
	FILE* f = fopen("Crawl.class", "rb");

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
				[&](auto attribute_reader) {
					return attribute_reader(
						[&](auto index){ return utf8_strings[index]; },
						[&](auto ) {
							
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
				[&](auto attribute_reader) {
					return attribute_reader(
						[&](auto index){ return utf8_strings[index]; },
						[&](auto ) {
							
						}
					);
				}
			);
		}
	);

}