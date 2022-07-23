#pragma once

#ifdef __x86_64__

#include "./decl.hpp"

#include "execution/stack_entry.hpp"
#include "abort.hpp"

#include <core/bit_cast.hpp>
#include <stdio.h>

typedef float __m128 __attribute__((__vector_size__(16), __aligned__(16)));
typedef double __m128d __attribute__((__vector_size__(16), __aligned__(16)));

inline stack_entry native_function::call(span<stack_entry, uint16> args) {
	if(args.size() > 5) {
		fputs("args.size() > 5", stderr);
		abort();
	}

	uint64 iorref_storage[args.size() + 1]; {
		nuint arg = 0;

		iorref_storage[arg++] = (uint64) nullptr; // jni_environment*

		for(stack_entry& se : args) {
			se.view([&]<typename Type>(Type& value) {
				if constexpr(same_as<Type, reference>) {
					iorref_storage[arg++] =
						bit_cast<uint64>(value.object_ptr());
				}
				else if constexpr(same_as<Type, jlong>) {
					iorref_storage[arg++] =
						bit_cast<uint64>(value);
				}
				else if constexpr(
					same_as<Type, jbool> || same_as<Type, jbyte> ||
					same_as<Type, jchar> || same_as<Type, jshort> ||
					same_as<Type, jint>
				) {
					iorref_storage[arg++] = (int32) (int64) value;
				}
			});
		}
	}

	__m128 floating_storage[8] { 0 }; {
		nuint arg = 0;
		for(stack_entry& se : args) {
			se.view([&]<typename Type>(Type& value) {
				if constexpr(same_as<Type, jfloat>) {
					floating_storage[arg++] =
						__extension__ (__m128){ value, 0, 0, 0 };
				}
				if constexpr(same_as<Type, jdouble>) {
					floating_storage[arg++] =
						__extension__ (__m128d){ value, 0 };
				}
			});
		}
	}

	register uint64 result asm("rax");
	register __m128 arg_f_0 asm("xmm0") = floating_storage[0];
	{
		register uint64 arg_0 asm("rdi") = iorref_storage[0];
		register uint64 arg_1 asm("rsi") = iorref_storage[1];
		register uint64 arg_2 asm("rdx") = iorref_storage[2];
		register uint64 arg_3 asm("rcx") = iorref_storage[3];
		register uint64 arg_4 asm("r8")  = iorref_storage[4];
		register uint64 arg_5 asm("r9")  = iorref_storage[5];

		register __m128 arg_f_1 asm("xmm1") = floating_storage[1];
		register __m128 arg_f_2 asm("xmm2") = floating_storage[2];
		register __m128 arg_f_3 asm("xmm3") = floating_storage[3];
		register __m128 arg_f_4 asm("xmm4") = floating_storage[4];
		register __m128 arg_f_5 asm("xmm5") = floating_storage[5];
		register __m128 arg_f_6 asm("xmm6") = floating_storage[6];
		register __m128 arg_f_7 asm("xmm7") = floating_storage[7];

		register uint64 function_ptr asm("rbx")  = (uint64) ptr_;

		asm volatile(
			"callq *%[function_ptr]\n"
			:
				[function_ptr]"+r"(function_ptr),
				"+r"(arg_0), "+r"(arg_1), "+r"(arg_2),
				"+r"(arg_3), "+r"(arg_4), "+r"(arg_5),
				"+r"(arg_f_0), "+r"(arg_f_1), "+r"(arg_f_2), "+r"(arg_f_3),
				"+r"(arg_f_4), "+r"(arg_f_5), "+r"(arg_f_6), "+r"(arg_f_7),
				"=r"(result)
		);
	}

	stack_entry se_result; {
		using namespace class_file::descriptor;

		method_reader params_reader{ desc_.data() };
		auto [return_type_reader, reading_result] =
			params_reader.skip_parameters();
		if(!reading_result) {
			abort();
		}
		return_type_reader([&]<typename Type>(Type){
			if constexpr(
				same_as<Type, Z> ||
				same_as<Type, C> || same_as<Type, S> ||
				same_as<Type, I>
			) {
				se_result = jint{ (int32) result };
				return true;
			}
			else if constexpr(
				same_as<Type, J>
			) {
				se_result = jlong{ (int64) result };
				return true;
			}
			else if constexpr(
				same_as<Type, F>
			) {
				se_result = jfloat{ arg_f_0[0] };
				return true;
			}
			else if constexpr(
				same_as<Type, D>
			) {
				se_result = jdouble{ ((__m128d) arg_f_0)[0] };
				return true;
			}
			else if constexpr(
				same_as<Type, object_type> || same_as<Type, array_type>
			) {
				se_result = reference{ * (object*) result };
				return true;
			}
			return false;
		});
	}

	return se_result;
}

#endif