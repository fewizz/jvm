#ifdef __x86_64__

#include "./for_each_parameter.hpp"

#include "decl/execute.hpp"
#include "decl/method.hpp"
#include "decl/native/thrown.hpp"

#include <numbers.hpp>

#include <class_file/descriptor/method_reader.hpp>

#include <posix/abort.hpp>

typedef float __m128 __attribute__((__vector_size__(16), __aligned__(16)));
typedef double __m128d __attribute__((__vector_size__(16), __aligned__(16)));

inline optional<reference>
try_native_interface_call(native_function_ptr ptr, method& m) {
	nuint jstack_begin = stack.size() - m.parameters_stack_size();

	uint64 i_regs[6]{};
	__m128 f_regs[8]{};

	uint8 stack_count = [&] {
		uint8 arg = 0;
		uint8 i_stack_count = 0;
		uint8 f_stack_count = 0;
		for_each_parameter(m, jstack_begin, [&]<typename Type>(Type) {
			if constexpr(same_as<Type, float> || same_as<Type, double>) {
				if(arg < 8) {
					++f_stack_count;
				}
			}
			else if(arg < 6) {
				++i_stack_count;
			}
			++arg;
		});
		return i_stack_count + f_stack_count;
	}();


	uint8 stack_storage_size = numbers{ uint8(1), stack_count }.max();
	uint64 stack_storage[stack_storage_size];
	for(nuint i = 0; i < stack_storage_size; ++i) { stack_storage[i] = 0; }

	{
		nuint arg = 0;

		for_each_parameter(m, jstack_begin, [&]<typename Type>(Type x) {
			if constexpr(same_as<Type, void*>) {
				(arg >= 6 ? stack_storage[arg - 6] : i_regs[arg]) =
					(uint64) x;
			}
			else if constexpr(same_as<Type, object*>) {
				(arg >= 6 ? stack_storage[arg - 6] : i_regs[arg]) =
					(uint64) x;
			}
			else if constexpr(same_as<Type, int32> || same_as<Type, int64>) {
				(arg >= 6 ? stack_storage[arg - 6] : i_regs[arg]) =
					(uint64) x;
			}
			else if constexpr(same_as<Type, float>) {
				if(arg >= 8) {
					stack_storage[arg - 8] = bit_cast<uint32>(x);
				}
				else {
					f_regs[arg] = __extension__ (__m128){ x, 0, 0, 0 };
				}
			}
			else if constexpr(same_as<Type, double>) {
				if(arg >= 8) {
					stack_storage[arg - 8] = bit_cast<uint64>(x);
				}
				else {
					f_regs[arg] = __extension__ (__m128d){ x, 0 };
				}
			} else {
				posix::abort();
			}
			++arg;
		});
	}

	register uint64 result asm("rax") = 0;
	register __m128 arg_0_f asm("xmm0") = f_regs[0];
	{
		register uint64 stack_remaining asm("rbx") = stack_count;

		register uint64 arg_0 asm("rdi") = i_regs[0];
		register uint64 arg_1 asm("rsi") = i_regs[1];
		register uint64 arg_2 asm("rdx") = i_regs[2];
		register uint64 arg_3 asm("rcx") = i_regs[3];
		register uint64 arg_4 asm("r8")  = i_regs[4];
		register uint64 arg_5 asm("r9")  = i_regs[5];

		register __m128 arg_1_f asm("xmm1") = f_regs[1];
		register __m128 arg_2_f asm("xmm2") = f_regs[2];
		register __m128 arg_3_f asm("xmm3") = f_regs[3];
		register __m128 arg_4_f asm("xmm4") = f_regs[4];
		register __m128 arg_5_f asm("xmm5") = f_regs[5];
		register __m128 arg_6_f asm("xmm6") = f_regs[6];
		register __m128 arg_7_f asm("xmm7") = f_regs[7];

		register uint64* stack_beginning asm("r11")  = stack_storage;
		register void* function_ptr asm("r12") = ptr;

		void* rsp_beginning = nullptr;

		asm volatile(
				"movq %%rsp, %%rax\n"
				"movq %%rax, %[rsp_beginning]\n"
				// alignment to 16
				"movq %[stack_remaining], %%rax\n"
				"salq $3, %%rax\n" // rax * 8
				"addq %%rsp, %%rax\n"
				"movq $0x10, %%r10\n" // r10 = 0x10
				"subq %%rax, %%r10\n" // r10 -= rax
				"andq $0xF,  %%r10\n" // r10 &= 0xF
				"addq %%r10, %%rsp\n" // rsp += rax
			"loop_begin:\n"
				"cmpq $0, %[stack_remaining]\n"
				"je loop_end\n"
				"subq $1, %[stack_remaining]\n"
				"movq 0(%[stack_beginning], %[stack_remaining], 8), %%rax\n"
				"pushq %%rax\n"
				"jmp loop_begin\n"
			"loop_end:\n"
				"callq *%[function_ptr]\n"
				"movq %[rsp_beginning], %%rsp\n"
			:
				"=r"(result),
				"+r"(arg_1), "+r"(arg_0), "+r"(arg_2),
				"+r"(arg_3), "+r"(arg_4), "+r"(arg_5),
				"+r"(arg_0_f), "+r"(arg_1_f), "+r"(arg_2_f), "+r"(arg_3_f),
				"+r"(arg_4_f), "+r"(arg_5_f), "+r"(arg_6_f), "+r"(arg_7_f),
				[rsp_beginning]"=m"(rsp_beginning),
				[stack_remaining]"+r"(stack_remaining)
			:
				[stack_beginning]"r"(stack_beginning),
				[function_ptr]"r"(function_ptr)
			: "r10", "r13", "r13", "r14", "r15", "memory", "cc"
		);
	}

	if(!thrown_in_native.is_null()) {
		return move(thrown_in_native);
	}

	m.return_type().view(
		[&]<typename Type>(Type) {
			if constexpr(same_as<Type, class_file::v>) {
				stack.pop_back_until(jstack_begin);
			}
			else if constexpr(
				same_as<Type, class_file::z> ||
				same_as<Type, class_file::b> ||
				same_as<Type, class_file::c> ||
				same_as<Type, class_file::s> ||
				same_as<Type, class_file::i>
			) {
				stack.pop_back_until(jstack_begin);
				stack.emplace_back<int32>((int32)(uint32) result);
			}
			else if constexpr(same_as<Type, class_file::j>) {
				stack.pop_back_until(jstack_begin);
				stack.emplace_back<int64>((int64) result);
			}
			else if constexpr(same_as<Type, class_file::f>) {
				stack.pop_back_until(jstack_begin);
				stack.emplace_back<float>(arg_0_f[0]);
			}
			else if constexpr(same_as<Type, class_file::d>) {
				stack.pop_back_until(jstack_begin);
				stack.emplace_back<double>(((__m128d) arg_0_f)[0]);
			}
			else if constexpr(
				same_as<Type, class_file::object>
			) {
				// increment reference count before possible deletion on stack
				object* obj_ptr = (::object*) result;
				reference ref =
					obj_ptr == nullptr ?
					reference{} :
					reference { * (::object*) result };

				stack.pop_back_until(jstack_begin);
				stack.emplace_back(move(ref));
			} else {
				posix::abort();
			}
		}
	);

	return {};
}

#endif