#pragma once

#include "../../execute/stack_entry.hpp"
#include "../../../alloc.hpp"
#include "../../../abort.hpp"
#include <core/c_string.hpp>
#include <core/range.hpp>
#include <core/copy.hpp>
#include <core/span.hpp>
#include <core/exchange.hpp>
#include <stdio.h>

struct native_function {
private:
	void* ptr_;
	c_string<c_string_type::known_size> name_;
	c_string<c_string_type::known_size> desc_;
public:

	template<range Name, range Descriptor>
	native_function(void* ptr, Name&& name, Descriptor&& desc) :
		ptr_{ ptr },
		name_{ (char*) default_allocator{}.allocate(name.size()), name.size() },
		desc_{ (char*) default_allocator{}.allocate(desc.size()), desc.size() }
	{
		span name0 {
			(char*) default_allocator{}.allocate(name.size()), name.size()
		};
		span desc0 {
			(char*) default_allocator{}.allocate(desc.size()), desc.size()
		};

		copy{ name }.to(name0);
		copy{ desc }.to(desc0);

		name_ = c_string{ (const char*) name0.data(), name0.size() };
		desc_ = c_string{ (const char*) desc0.data(), desc0.size() };
	}

	~native_function() {
		default_allocator{}.deallocate((uint8*) name_.data(), name_.size());
		default_allocator{}.deallocate((uint8*) desc_.data(), desc_.size());
	}

	native_function(const native_function&) = delete;
	native_function& operator = (const native_function&) = delete;

	operator void* () { return ptr_; }

	template<typename ReturnType>
	stack_entry call(span<stack_entry, uint16> args);

	auto name() { return name_; }
};

#include "../jni/environment.hpp"
#include <core/bit_cast.hpp>
#include <class/file/descriptor/reader.hpp>

template<typename ReturnType>
inline stack_entry native_function::call(span<stack_entry, uint16> args) {
	uint64 args_storage[args.size()];
	nuint arg = 0;
	for(stack_entry& se : args) {
		se.view([&]<typename Type>(Type& value) {
			if constexpr(same_as<Type, reference>) {
				args_storage[arg++] = bit_cast<uint64>(value.object_ptr());
			}
			else if constexpr(same_as<Type, jlong> || same_as<Type, jdouble>) {
				args_storage[arg++] = bit_cast<uint64>(value);
			}
			else if constexpr(same_as<Type, jfloat>) {
				args_storage[arg++] = bit_cast<uint32>(value);
			}
			else if constexpr(
				same_as<Type, jbool> || same_as<Type, jbyte> ||
				same_as<Type, jchar> || same_as<Type, jshort> ||
				same_as<Type, jint>
			) {
				args_storage[arg++] = (int32) (int64) value;
			}
		});
	}

	using namespace class_file::descriptor;

	uint8 types_storage[args.size()];
	nuint type = 0;
	method_reader params_reader{ desc_.data() };

	auto param_to_id = []<typename Type>(Type, uint8& id) {
		if constexpr(
			same_as<Type, B> || same_as<Type, S> || same_as<Type, C> ||
			same_as<Type, I> || same_as<Type, Z>
		) {
			id = 0;
			return true;
		}
		else if constexpr(
			same_as<Type, J> ||
			same_as<Type, object_type> || same_as<Type, array_type>
		) {
			id = 1;
			return true;
		}
		else if constexpr(same_as<Type, F> || same_as<Type, D>) {
			id = 2;
			return true;
		}
		else if constexpr(same_as<Type, V>) {
			id = 3;
			return true;
		}
		return false;
	};

	auto [ret_type_reader, result0] = params_reader([&](auto t) {
		return param_to_id(t, types_storage[type++]);
	});

	uint8 return_type_id;
	ret_type_reader([&](auto t) {
		return param_to_id(t, return_type_id);
	});

	register uint32 args_count asm("w9") = (uint32) args.size();
	register uint64 args_ptr asm("x10") = (uint64) args_storage;
	register uint64 types_ptr asm("x11") = (uint64) types_storage;
	register uint64 function_ptr asm("x12") = (uint64) ptr_;
	register uint32 ret_type_id0 asm("x13") = return_type_id;
	register uint64 result asm("x14");

	asm volatile(
		"mov w17, 0\n"
		"mov w18, 0\n"

	"1:\n"
		"cmp %w[args_count], wzr\n"
		"beq 3f\n"

		// 32-bit
		"ldrb w15, [%x[types_ptr]]\n"
		"cmp w15, 0\n"
		"beq 11f\n"

		// 64-bit
		"cmp w15, 1\n"
		"beq 12f\n"

		// floating-point
		//"b 13f\n"

		"11:\n"
			"cmp w17, wzr\n"
			"beq 101f\n"
			"cmp w17, 1\n"
			"beq 102f\n"
			"cmp w17, 2\n"
			"beq 103f\n"
			"cmp w17, 3\n"
			"beq 104f\n"
			"cmp w17, 4\n"
			"beq 105f\n"
			"cmp w17, 5\n"
			"beq 106f\n"
			"cmp w17, 6\n"
			"beq 107f\n"
			"cmp w17, 7\n"
			"beq 108f\n"

			"101:\n"
				"ldr w0, [%x[args_ptr]]\n"
				"b 109f\n"
			"102:\n"
				"ldr w1, [%x[args_ptr]]\n"
				"b 109f\n"
			"103:\n"
				"ldr w2, [%x[args_ptr]]\n"
				"b 109f\n"
			"104:\n"
				"ldr w3, [%x[args_ptr]]\n"
				"b 109f\n"
			"105:\n"
				"ldr w4, [%x[args_ptr]]\n"
				"b 109f\n"
			"106:\n"
				"ldr w5, [%x[args_ptr]]\n"
				"b 109f\n"
			"107:\n"
				"ldr w6, [%x[args_ptr]]\n"
				"b 109f\n"
			"108:\n"
				"ldr w7, [%x[args_ptr]]\n"
				"b 109f\n"
			"109:\n"
				"add w17, w17, 1\n"
				"b 2f\n"
		"12:\n"
			"cmp w17, wzr\n"
			"beq 201f\n"
			"cmp w17, 1\n"
			"beq 202f\n"
			"cmp w17, 2\n"
			"beq 203f\n"
			"cmp w17, 3\n"
			"beq 204f\n"
			"cmp w17, 4\n"
			"beq 205f\n"
			"cmp w17, 5\n"
			"beq 206f\n"
			"cmp w17, 6\n"
			"beq 207f\n"
			"cmp w17, 7\n"
			"beq 208f\n"

			"201:\n"
				"ldr x0, [%x[args_ptr]]\n"
				"b 209f\n"
			"202:\n"
				"ldr x1, [%x[args_ptr]]\n"
				"b 209f\n"
			"203:\n"
				"ldr x2, [%x[args_ptr]]\n"
				"b 209f\n"
			"204:\n"
				"ldr x3, [%x[args_ptr]]\n"
				"b 209f\n"
			"205:\n"
				"ldr x4, [%x[args_ptr]]\n"
				"b 209f\n"
			"206:\n"
				"ldr x5, [%x[args_ptr]]\n"
				"b 209f\n"
			"207:\n"
				"ldr x6, [%x[args_ptr]]\n"
				"b 209f\n"
			"208:\n"
				"ldr x7, [%x[args_ptr]]\n"
				"b 209f\n"
			"209:\n"
				"add w17, w17, 1\n"
				"b 2f\n"

	"2:\n"
		"sub %w[args_count], %w[args_count], 1\n"
		"add %x[args_ptr], %x[args_ptr], 8\n"
		"add %x[types_ptr], %x[types_ptr], 1\n"
		"b 1b\n"
	"3:\n"
		"stp x29, x30, [sp, -16]!\n"
		"mov x29, sp\n"
		"blr %x[function_ptr]\n"
		"ldp x29, x30, [sp], 16\n"

		"cmp %w[ret_type_id], 0\n"
		"beq 31f\n"

		"cmp %w[ret_type_id], 1\n"
		"beq 32f\n"

		"cmp %w[ret_type_id], 2\n"
		"beq 33f\n"

		"cmp %w[ret_type_id], 3\n"
		"beq 34f\n"

		"31:\n"
			"mov %w[result], w0\n"
			"b 4f\n"

		"32:\n"
			"mov %x[result], x0\n"
			"b 4f\n"

		"33:\n"
			// TODO
			"b 4f\n"
		
		"34:\n"
			"mov %x[result], xzr\n"
			"b 4f\n"
	"4:"
		:
			[args_count]"+r"(args_count),
			[args_ptr]"+r"(args_ptr),
			[types_ptr]"+r"(types_ptr),
			[result]"=r"(result)
		:
			[ret_type_id]"r"(ret_type_id0),
			[function_ptr]"r"(function_ptr)
		: "w15", "w16", "cc"
	);

	stack_entry se;

	ret_type_reader([&]<typename Type>(Type) {
		if constexpr(
			same_as<Type, B> || same_as<Type, S> || same_as<Type, C> ||
			same_as<Type, I> || same_as<Type, Z>
		) {
			se = jint( int32(result) );
			return true;
		}
		else if constexpr(same_as<Type, F>) {
			se = jfloat(bit_cast<float>((uint32) result));
			return true;
		}
		else if constexpr(same_as<Type, J>) {
			se = jlong(bit_cast<int64>(result));
			return true;
		}
		else if constexpr(same_as<Type, D>) {
			se = jdouble(bit_cast<double>(result));
			return true;
		}
		else if constexpr(
			same_as<Type, object_type> || same_as<Type, array_type>
		) {
			se = reference{ *((object*) result) };
			return true;
		}
		else if constexpr(same_as<Type, V>) {
			se = jvoid{};
			return true;
		}
		return false;
	});

	return se;
}