#ifdef __x86_64__

#include "decl/execution/stack_entry.hpp"
#include "decl/execute.hpp"
#include "decl/method.hpp"
#include "decl/abort.hpp"

#include <max.hpp>

#include <stdio.h>

typedef float __m128 __attribute__((__vector_size__(16), __aligned__(16)));
typedef double __m128d __attribute__((__vector_size__(16), __aligned__(16)));

template<typename Descriptor>
inline optional<stack_entry> native_interface_call(
	native_function_ptr ptr, arguments_span args, Descriptor&& descriptor
) {
	uint64 iorref_storage[4]   { 0 };
	__m128 floating_storage[4] { 0 };
	nuint stack_size = (max(4, args.size() + 1) - 4);
	if(stack_size % 2 != 0) { // 8 byte element, size aligned to 16
		++stack_size;
	}
	uint64 stack_storage[stack_size];
	
	{
		nuint arg = 0;

		// native_interface_environment*
		iorref_storage[arg++] = (uint64) nullptr;

		for(stack_entry& se : args) {
			se.view([&]<typename Type>(Type& value) {
				if constexpr(same_as<Type, reference>) {
					(arg >= 4 ? stack_storage[arg - 4] : iorref_storage[arg]) =
						bit_cast<uint64>(value.object_ptr());
				}
				if constexpr(same_as<Type, jlong>) {
					(arg >= 4 ? stack_storage[arg - 4] : iorref_storage[arg]) =
						bit_cast<uint64>(value);
				}
				if constexpr(same_as<Type, jint>) {
					(arg >= 4 ? stack_storage[arg - 4] : iorref_storage[arg]) =
						(int32) (int64) value;
				}
				if constexpr(same_as<Type, jfloat>) {
					if(arg >= 4) {
						stack_storage[arg - 4] = bit_cast<uint32>(value);
					}
					else {
						floating_storage[arg++] =
							__extension__ (__m128){ value, 0, 0, 0 };
					}
				}
				if constexpr(same_as<Type, jdouble>) {
					floating_storage[arg++] =
						__extension__ (__m128d){ value, 0 };
				}
			});
			++arg;
		}
	}

	register uint64 result asm("rax");
	register __m128 arg_f_0 asm("xmm0") = floating_storage[0];
	{
		register uint64 stack_remaining asm("rbx") = stack_size;
		register uint64 stack_beginning asm("rsi") = (uint64) stack_storage;

		register uint64 arg_1 asm("rcx") = iorref_storage[0];
		register uint64 arg_0 asm("rdx") = iorref_storage[1];
		register uint64 arg_2 asm("r8")  = iorref_storage[2];
		register uint64 arg_3 asm("r9")  = iorref_storage[3];

		register __m128 arg_f_1 asm("xmm1") = floating_storage[1];
		register __m128 arg_f_2 asm("xmm2") = floating_storage[2];
		register __m128 arg_f_3 asm("xmm3") = floating_storage[3];

		register uint64 function_ptr asm("rdi")  = (uint64) (void*) ptr;

		asm volatile(
				"movq %[stack_remaining], %%r10\n" // save `stack_remaining`
			"loop_begin:\n"
				"cmpq $0, %[stack_remaining]\n"
				"je loop_end\n"
				"subq $1, %[stack_remaining]\n"
				"movq 0(%[stack_beginning], %[stack_remaining], 8), %%rax\n"
				"pushq %%rax\n"
				"jmp loop_begin\n"
			"loop_end:\n"
				"sub $16, %%rsp\n"
				"pushq %[stack_beginning]\n"
				"pushq %%r10\n" // push `stack_remaining`
				"callq *%[function_ptr]\n"
				"popq %[stack_remaining]\n"
				"popq %[stack_beginning]\n"
				"addq $16, %%rsp\n"
				"shlq $3, %[stack_remaining]\n"
				"addq %[stack_remaining], %%rsp\n"
			:
				[function_ptr] "+r"(function_ptr),
				"+r"(arg_0),   "+r"(arg_1),   "+r"(arg_2),   "+r"(arg_3),
				"+r"(arg_f_0), "+r"(arg_f_1), "+r"(arg_f_2), "+r"(arg_f_3),
				[stack_remaining]"+r"(stack_remaining),
				[stack_beginning]"+r"(stack_beginning),
				"+r"(result)
			:
			:
				"r10", "r11", "r12", "r13", "r14", "r15",
				"cc", "memory"
		);
	}

	stack_entry se_result = reference{}; {
		using namespace class_file;

		descriptor::method::reader reader {
			range_iterator(descriptor)
		};
		auto return_type_reader =
			reader.try_skip_parameters_and_get_return_type_reader(
				[](auto){ abort(); }
			).value();
		return_type_reader.try_read_and_get_advanced_iterator(
			[&]<typename Type>(Type) {
				if constexpr(
					same_as<Type, descriptor::z> ||
					same_as<Type, descriptor::c> ||
					same_as<Type, descriptor::s> ||
					same_as<Type, descriptor::i>
				) {
					se_result = jint{ (int32) result };
					return true;
				}
				else if constexpr(
					same_as<Type, descriptor::j>
				) {
					se_result = jlong{ (int64) result };
					return true;
				}
				else if constexpr(
					same_as<Type, descriptor::f>
				) {
					se_result = jfloat{ arg_f_0[0] };
					return true;
				}
				else if constexpr(
					same_as<Type, descriptor::d>
				) {
					se_result = jdouble{ ((__m128d) arg_f_0)[0] };
					return true;
				}
				else if constexpr(
					same_as<Type, descriptor::object> ||
					same_as<Type, descriptor::array>
				) {
					se_result = reference{ * (object*) result };
					return true;
				}
				return false;
			},
			[](auto){ abort(); }
		);
	}

	return se_result;
}

#endif