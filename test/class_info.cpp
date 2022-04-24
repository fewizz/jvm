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

#include "class.hpp"
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

	class_reader magic_reader{ data };
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

			if constexpr (same_as<Type, utf8_info>) {
				printf("utf8: ");
				fwrite(x.begin(), 1, x.size(), stdout);
				printf("\n");
			}
			else if constexpr (same_as<Type, integer_info>) {
				printf("integer: value: %d\n", x.value);
			}
			else if constexpr (same_as<Type, class_info>) {
				printf("class: name index: %d\n", x.name_index);
			}
			else if constexpr (same_as<Type, string_info>) {
				printf("string: index: %d\n", x.string_index);
			}
			else if constexpr (same_as<Type, fieldref_info>) {
				printf(
					"fieldref: class index: %d, name and type index: %d\n",
					x.class_index,
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, methodref_info>) {
				printf(
					"methodref: class index: %d, name and type index: %d\n",
					x.class_index,
					x.name_and_type_index
				);
			}
			else if constexpr (same_as<Type, name_and_type_info>) {
				printf(
					"name and type: name index: %d, descriptor index: %d\n",
					x.name_index,
					x.descriptor_index
				);
			}
			else if constexpr (same_as<Type, method_handle_info>) {
				printf(
					"method handle: reference kind: %d, reference index: %d\n",
					x.reference_kind, x.reference_index
				);
			}
			else if constexpr (same_as<Type, method_type_info>) {
				printf(
					"method type: descriptor_index: %d\n",
					x.descriptor_index
				);
			}
			else if constexpr (same_as<Type, invoke_dynamic_info>) {
				printf(
					"invoke dynamic: bootstrap method attr index: %d",
					x.bootstrap_method_attr_index
				);

				printf(
					", name_and_type_index: %d\n",
					x.name_and_type_index
				);
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

	utf8_info utf8_strings[constant_pool_entry_index];
	class_info classes[constant_pool_entry_index];

	constant_pool_entry_index = 1;

	constant_pool_reader(
		[&]<typename Type>(Type x) {
			if constexpr (same_as<Type, utf8_info>) {
				utf8_strings[constant_pool_entry_index] = x;
			}
			if constexpr (same_as<Type, class_info>) {
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
		[&](field_info fi, auto attributes_reader) {
			printf("\taccess flags: 0x%.4x, name: ", fi.access_flags);
			fwrite(
				utf8_strings[fi.name_index].begin(),
				1,
				utf8_strings[fi.name_index].size(),
				stdout
			);

			printf(", descriptor: ");
			fwrite(
				utf8_strings[fi.descriptor_index].begin(),
				1,
				utf8_strings[fi.descriptor_index].size(),
				stdout
			);

			printf("\n");

			attributes_reader.read_attributes(
				[&](attribute_info attribute_info) {
					printf("\t\tname: ");
					fwrite(
						utf8_strings[attribute_info.name_index].begin(),
						1,
						utf8_strings[attribute_info.name_index].size(),
						stdout
					);
					printf("\n");
				}
			);
		}
	);

	printf("methods:\n");
	auto attributes_reader = methods_reader(
		[&](method_info mi, auto attributes_reader) {
			printf("\taccess flags: 0x%.4x, name: ", mi.access_flags);
			fwrite(
				utf8_strings[mi.name_index].begin(),
				1,
				utf8_strings[mi.name_index].size(),
				stdout
			);

			printf(", descriptor: ");
			fwrite(
				utf8_strings[mi.descriptor_index].begin(),
				1,
				utf8_strings[mi.descriptor_index].size(),
				stdout
			);

			printf(", attributes:\n");

			attributes_reader.read_attributes(
				[&](attribute_info attribute_info) {
					auto name = utf8_strings[attribute_info.name_index];
					printf("\t\tname: ");
					fwrite(name.begin(), 1, name.size(), stdout);
					printf("\n");

					if(range::equals(name, array{'C', 'o', 'd', 'e'})) {
						code_attribute_reader max_locals_reader {
							attribute_info.data.begin(),
							attribute_info.data.end()
						};

						auto[max_stack_reader, max_locals] = max_locals_reader();

						printf("max locals: %d\n", max_locals);

						auto[code_reader, max_stack] = max_stack_reader();

						printf("max stack: %d\n", max_locals);
					}
				}
			);
		}
	);

}