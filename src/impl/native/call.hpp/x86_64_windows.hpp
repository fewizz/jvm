#ifdef __x86_64__

#include "./for_each_parameter.hpp"

#include "decl/execute.hpp"
#include "decl/method.hpp"
#include "decl/execution/info.hpp"
#include "decl/native/thrown.hpp"

#include <class_file/descriptor/method.hpp>
#include <overloaded.hpp>
#include <types.hpp>
#include <numbers.hpp>

#include <posix/abort.hpp>

typedef float __m128 __attribute__((__vector_size__(16), __aligned__(16)));
typedef double __m128d __attribute__((__vector_size__(16), __aligned__(16)));

inline optional<reference>
try_native_interface_call(native_function_ptr ptr, method& m) {
	nuint jstack_begin = stack.size() - m.parameters_stack_size();

	uint64 i_regs[4]{};
	__m128 f_regs[4]{};

	uint8 stack_count = [&] {
		uint8 arg = 0;
		uint8 i_stack_count = 0;
		uint8 f_stack_count = 0;
		for_each_parameter(m, jstack_begin, [&]<typename Type>(Type) {
			if(arg >= 4) { return; }
			if constexpr(same_as<Type, float> || same_as<Type, double>) {
				++f_stack_count;
			}
			else {
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

		for_each_parameter(m, jstack_begin, overloaded {
			[&](auto* ptr) {
				(arg >= 4 ? stack_storage[arg - 4] : i_regs[arg]) =
					(uint64) ptr;
			},
			[&]<same_as_any<int32, int64> Type>(Type x) {
				(arg >= 4 ? stack_storage[arg - 4] : i_regs[arg]) = (uint64) x;
			},
			[&](float f) {
				if(arg >= 4) {
					stack_storage[arg - 4] = bit_cast<uint32>(f);
				}
				else {
					f_regs[arg] = __extension__ (__m128){ f, 0, 0, 0 };
				}
			},
			[&](double d) {
				if(arg >= 4) {
					stack_storage[arg - 4] = bit_cast<uint64>(d);
				}
				else {
					f_regs[arg] = __extension__ (__m128d){ d, 0 };
				}
			}
		}.then([&]{ ++arg; }));
	}

	register uint64 result asm("rax") = 0;
	register __m128 arg_0_f asm("xmm0") = f_regs[0];
	{
		register uint64 stack_remaining asm("rbx") = stack_count;

		register uint64 arg_0 asm("rcx") = i_regs[0];
		register uint64 arg_1 asm("rdx") = i_regs[1];
		register uint64 arg_2 asm("r8")  = i_regs[2];
		register uint64 arg_3 asm("r9")  = i_regs[3];

		register __m128 arg_1_f asm("xmm1") = f_regs[1];
		register __m128 arg_2_f asm("xmm2") = f_regs[2];
		register __m128 arg_3_f asm("xmm3") = f_regs[3];

		void* rsp_beginning = nullptr;

		register uint64* stack_beginning asm("r11")  = stack_storage;
		register void* function_ptr asm("r12") = ptr;

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
				"+r"(arg_1), "+r"(arg_0), "+r"(arg_2), "+r"(arg_3),
				"+r"(arg_0_f), "+r"(arg_1_f), "+r"(arg_2_f), "+r"(arg_3_f),
				[rsp_beginning]"=m"(rsp_beginning),
				[stack_remaining]"+r"(stack_remaining)
			:
				[stack_beginning]"r"(stack_beginning),
				[function_ptr]"r"(function_ptr)
			: "r10", "r13", "r14", "r15", "memory", "cc"
		);
	}

	if(!thrown_in_native.is_null()) {
		return move(thrown_in_native);
	}

	m.return_type().view(
		[&]<typename Type>(Type) {
			if constexpr(same_as<Type, class_file::v>) {
				stack.erase_back_until(jstack_begin);
			}
			else if constexpr(
				same_as<Type, class_file::z> ||
				same_as<Type, class_file::b> ||
				same_as<Type, class_file::c> ||
				same_as<Type, class_file::s> ||
				same_as<Type, class_file::i>
			) {
				stack.erase_back_until(jstack_begin);
				stack.emplace_back<int32>((int32)(uint32) result);
			}
			else if constexpr(same_as<Type, class_file::j>) {
				stack.erase_back_until(jstack_begin);
				stack.emplace_back<int64>((int64) result);
			}
			else if constexpr(same_as<Type, class_file::f>) {
				stack.erase_back_until(jstack_begin);
				stack.emplace_back<float>(arg_0_f[0]);
			}
			else if constexpr(same_as<Type, class_file::d>) {
				stack.erase_back_until(jstack_begin);
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

				stack.erase_back_until(jstack_begin);
				stack.emplace_back(move(ref));
			} else {
				[]<bool b = false>{ static_assert(b); };
			}
		}
	);

	return {};
}

#endif