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
	-o ${d}/../test/jvm \
	${d}/jvm.cpp

${d}/../test/jvm $@

exit 0
#endif

#include "class/file/constant.hpp"
#include "class/file/reader.hpp"
#include "class/instane.hpp"
#include "alloc.hpp"

#include <stdio.h>
#include <core/meta/type/is_same_as.hpp>

extern "C" void abort();

fixed_vector<class_file::constant::utf8, nuint, default_allocator>
	utf8_constants_storage { 65536 };
fixed_vector<class_file::constant::utf8, nuint, default_allocator>
	utf8_constatns { 65536 };
fixed_vector<_class, nuint, default_allocator>
	classes{ 65536 };

template<typename It>
elements::of<
	class_file::field::reader<It, class_file::field::reader_stage::end>,
	field
> load_field(
	class_file::field::reader<It> access_reader
) {
	auto [name_index_reader, access_flags] = access_reader();
	auto [descriptor_reader, name_index] = name_index_reader();
	auto [attributes_reader, descriptor_index] = descriptor_reader();

	auto end = attributes_reader([&](class_file::attribute::reader<It> attr) {
		return attr.skip();
	});

	return {
		end,
		field {
			access_flags,
			name_index,
			descriptor_index
		}
	};
}

template<typename It>
elements::of<
	class_file::method::reader<It, class_file::method::reader_stage::end>,
	method
> load_method(
	auto mapper,
	class_file::method::reader<It> access_reader
) {
	auto [name_index_reader, access_flags] = access_reader();
	auto [descriptor_reader, name_index] = name_index_reader();
	auto [attributes_reader, descriptor_index] = descriptor_reader();

	code code;

	auto end = attributes_reader([&](class_file::attribute::reader<It> attr) {
		return attr(
			mapper,
			[&]<typename Type>(Type x) {
				if constexpr (
					same_as<
						Type,
						class_file::attribute::code::reader<It>
					>
				) {
					auto [max_locals_reader, max_stack] = x();
					auto [code_reader, max_locals] = x();

					auto src0 = code_reader.src;
					uint32 length = read<uint32, endianness::big>(src0);

					code = {
						max_stack,
						max_locals,
						{ src0, length }
					};
				}
			}
		);
	});

	return {
		end,
		method {
			access_flags,
			name_index,
			descriptor_index,
			code
		}
	};
}

_class load_class(const char* path) {
	FILE* f = fopen(path, "rb");
	if(f == nullptr) {
		printf("couldn't open class file %s", path);
		abort();
	}
	fseek(f, 0, SEEK_END);
	nuint size = ftell(f);
	rewind(f);
	uint8* data = (uint8*) malloc(size);
	fread(data, 1, size, f);
	fclose(f);

	class_file::reader magic_reader{ data };
	auto [version_reader, magic_exists] = magic_reader();
	if(!magic_exists) {
		printf("magic doesn't exist for %s", path);
		abort();
	}

	auto [constant_pool_reader, version] = version_reader();
	uint16 constant_pool_size = 0;
	constant_pool_reader([&](auto) { ++constant_pool_size; });

	fixed_vector<const_pool_entry, uint16, default_allocator> constant_pool {
		constant_pool_size
	};
	fixed_vector<void*, uint16, default_allocator> ptrs { constant_pool_size };

	auto access_flags_reader = constant_pool_reader([&]<typename Type>(Type x) {
		if constexpr(same_as<uint8, Type>) {
			printf("unknown constant %hhu", x);
		}
		else {
			constant_pool.push_back(x);
			ptrs.push_back(nullptr);
		}
	});

	auto [this_class_reader, access_flags] = access_flags_reader();
	auto [super_class_reader, this_class] = this_class_reader();
	auto [interfaces_reader, super_class] = super_class_reader();

	uint16 interfaces_count = 0;
	interfaces_reader([&](auto) { ++interfaces_count; });
	fixed_vector<uint16, uint16, default_allocator> interfaces {
		interfaces_count
	};
	auto fields_reader = interfaces_reader([&](auto x) {
		interfaces.push_back(move(x));
	});

	uint16 fields_count = fields_reader.count();

	fixed_vector<field, uint16, default_allocator> fields { fields_count };
	auto methods_reader = fields_reader(
		[&]<typename It>(class_file::field::reader<It> acc_reader) {
		auto [reader, f] = load_field(acc_reader);
		fields.push_back(move(f));
		return reader;
	});

	uint16 methods_count = methods_reader.count();
	fixed_vector<method, uint16, default_allocator> methods { methods_count };

	methods_reader([&]<typename It>(class_file::method::reader<It> acc_reader) {
		auto [reader, m] = load_method(
			[&](auto name) {
				return constant_pool[name - 1].template get<
					class_file::constant::utf8
				>();
			}, // mapper
			acc_reader
		);

		methods.push_back(move(m));

		return reader;
	});

	return {
		move(constant_pool),
		access_flags,
		this_class,
		super_class,
		move(interfaces),
		move(fields),
		move(methods),
		move(ptrs),
		data
	};
}

void execute(method& m) {

}

int main () {
	_class cls = load_class("java/lang/System.class");
	int i = 0;
}