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
#include <core/bit_cast.hpp>
#include <core/c_string.hpp>

extern "C" [[noreturn]] void abort();

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
					auto [code_reader, max_locals] = max_locals_reader();

					auto src0 = code_reader.src;
					uint32 length = read<uint32, endianness::big>(src0);

					code = ::code {
						max_stack,
						max_locals,
						{ code_reader.src, length }
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

method find_method(auto name, _class& c) {
	for(auto m : c.methods) {
		using namespace class_file::constant;
		auto possible = c.const_pool[m.name_index - 1].get<utf8>();
		if(equals(possible, name)) {
			return m;
		}
	}
	printf("couldn't find method %s", name);
	abort();
}

auto execute(_class& c, method m, span<uint32, uint16> args = {}) {
	class_file::attribute::code::reader<
		uint8*,
		class_file::attribute::code::reader_stage::code
	> reader{ m.code.code.data() };

	uint32 stack[m.code.max_stack];
	nuint stack_size = 0;

	uint32 local[m.code.max_locals];
	for(int i = 0; i < m.code.max_locals; ++i) local[0] = 0;
	for(int i = 0; i < args.size(); ++i) local[i] = args[i];

	using namespace class_file::code::instruction;

	uint32 result = 0;

	reader([&]<typename Type>(Type x, uint8*& pc) {
		if constexpr (same_as<Type, nop>) {}
		else if constexpr (same_as<Type, i_const_0>) {
			stack[stack_size++] = 0;
		}
		else if constexpr (same_as<Type, i_const_1>) {
			stack[stack_size++] = 1;
		}
		else if constexpr (same_as<Type, i_const_2>) {
			stack[stack_size++] = 2;
		}
		else if constexpr (same_as<Type, i_const_3>) {
			stack[stack_size++] = 3;
		}
		else if constexpr (same_as<Type, i_const_4>) {
			stack[stack_size++] = 4;
		}
		else if constexpr (same_as<Type, i_const_5>) {
			stack[stack_size++] = 5;
		}

		else if constexpr (same_as<Type, bi_push>) {
			stack[stack_size++] = int32(x.value);
		}
		else if constexpr (same_as<Type, ldc>) {
			auto& e = c.const_pool[x.index - 1];
			auto v = e.template get<class_file::constant::integer>();
			stack[stack_size++] = v.value; // even if it is float
		}

		else if constexpr (same_as<Type, i_load_0>) {
			stack[stack_size++] = local[0];
		}
		else if constexpr (same_as<Type, i_load_1>) {
			stack[stack_size++] = local[1];
		}

		else if constexpr (same_as<Type, i_store_0>) {
			local[0] = stack[--stack_size];
		}
		else if constexpr (same_as<Type, i_store_1>) {
			local[1] = stack[--stack_size];
		}

		else if constexpr (same_as<Type, i_return>) {
			result = stack[--stack_size];
			return true;
		}

		else if constexpr (same_as<Type, i_add>) {
			int32 value2 = stack[--stack_size];
			int32 value1 = stack[--stack_size];
			stack[stack_size++] = value1 + value2;
		}
		else if constexpr (same_as<Type, i_sub>) {
			int32 value2 = stack[--stack_size];
			int32 value1 = stack[--stack_size];
			stack[stack_size++] = value1 - value2;
		}
		else if constexpr (same_as<Type, i_mul>) {
			int32 value2 = stack[--stack_size];
			int32 value1 = stack[--stack_size];
			stack[stack_size++] = value1 * value2;
		}
		else if constexpr (same_as<Type, i_inc>) {
			local[x.index] += int32(x.value);
		}

		else if constexpr (same_as<Type, if_i_cmp_ge>) {
			int32 value2 = int32(stack[--stack_size]);
			int32 value1 = int32(stack[--stack_size]);
			if(value1 >= value2) {
				pc += x.branch - sizeof(int16) - sizeof(uint8);
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_gt>) {
			int32 value2 = int32(stack[--stack_size]);
			int32 value1 = int32(stack[--stack_size]);
			if(value1 > value2) {
				pc += x.branch - sizeof(int16) - sizeof(uint8);
			}
		}
		else if constexpr (same_as<Type, go_to>) {
			pc += x.branch - sizeof(int16) - sizeof(uint8);
		}

		else if constexpr (same_as<Type, invoke_static>) {
			auto e = c.const_pool[x.index - 1];
			auto r = e.template get<class_file::constant::methodref>();
			auto nat_e = c.const_pool[r.name_and_type_index - 1];
			auto nt = nat_e.template get<class_file::constant::name_and_type>();
			auto n_e = c.const_pool[nt.name_index - 1];
			auto n = n_e.template get<class_file::constant::utf8>();
			auto m = find_method(n, c);

			uint16 args = 1;
			stack_size -= args;
			stack[stack_size] = execute(
				c, m, span<uint32, uint16>{ stack + stack_size, args }
			);
			++stack_size;
		}

		else {
			printf("unimplemented");
			abort();
		}

	}, m.code.code.size());

	return result;
}

int main () {
	_class cls = load_class("recursive.class");
	printf("%d", execute(cls, find_method(c_string{ "_recursive" }, cls)));
}