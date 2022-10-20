#ifdef __x86_64__

#include "decl/execution/stack_entry.hpp"
#include "decl/execute.hpp"
#include "decl/method.hpp"

#include <max.hpp>
#include <class_file/descriptor/method_reader.hpp>

#include <posix/abort.hpp>

typedef float __m128 __attribute__((__vector_size__(16), __aligned__(16)));
typedef double __m128d __attribute__((__vector_size__(16), __aligned__(16)));

inline void native_interface_call(native_function_ptr ptr, method& m) {
	uint64 i64_storage[4]   { 0 };
	__m128 f_storage[4] { 0 };
	nuint stack_size = (max(4, m.parameters_count() + 1) - 4);
	if(stack_size % 2 != 0) { // 8 byte element, size aligned to 16
		++stack_size;
	}
	uint64 stack_storage[stack_size];

	nuint stack_begin = stack.size() - m.parameters_stack_size();

	{
		nuint arg = 0;
		// native_interface_environment*
		i64_storage[arg++] = (uint64) nullptr;

		nuint stack_at = stack_begin;

		auto put = [&](variant<reference, jint, jlong, jfloat, jdouble> pt) {
			pt.view([&]<typename Type>(Type& value) {
				if constexpr(same_as<Type, reference>) {
					(arg >= 4 ? stack_storage[arg - 4] : i64_storage[arg]) =
						(uint64) value.object_ptr();
				}
				else if constexpr(same_as<Type, jlong>) {
					(arg >= 4 ? stack_storage[arg - 4] : i64_storage[arg]) =
						(uint64) (int64) value;
				}
				else if constexpr(same_as<Type, jint>) {
					(arg >= 4 ? stack_storage[arg - 4] : i64_storage[arg]) =
						(uint64) (int32) value;
				}
				else if constexpr(same_as<Type, jfloat>) {
					if(arg >= 4) {
						stack_storage[arg - 4] = bit_cast<uint32>(value);
					}
					else {
						f_storage[arg++] =
							__extension__ (__m128){ value, 0, 0, 0 };
					}
				}
				else if constexpr(same_as<Type, jdouble>) {
					f_storage[arg++] =
						__extension__ (__m128d){ value, 0 };
				} else {
					abort();
				}
			});
			++arg;
		};

		if(!m.is_static()) { // this
			reference& r = stack.at<reference>(stack_at);
			put(r);
			stack_at += 1;
		}

		for(one_of_non_void_descriptor_types pt : m.parameter_types()) {
			pt.view([&]<typename Type>(Type) {
				if constexpr(
					same_as<Type, class_file::object> ||
					same_as<Type, class_file::array>
				) {
					reference& r = stack.at<reference>(stack_at);
					put(r);
					stack_at += 1;
				}
				else if constexpr(same_as<Type, class_file::j>) {
					int64 v = stack.at<int64>(stack_at);
					put(jlong{ v });
					stack_at += 2;
				}
				else if constexpr(
					same_as<Type, class_file::z> ||
					same_as<Type, class_file::b> ||
					same_as<Type, class_file::c> ||
					same_as<Type, class_file::s> ||
					same_as<Type, class_file::i>
				) {
					int32 v = stack.at<int32>(stack_at);
					put(jint{ v });
					stack_at += 1;
				}
				else if constexpr(same_as<Type, class_file::f>) {
					float v = stack.at<float>(stack_at);
					put(jfloat{ v });
					stack_at += 1;
				}
				else if constexpr(same_as<Type, class_file::d>) {
					double v = stack.at<double>(stack_at);
					put(jdouble{ v });
					stack_at += 2;
				} else {
					abort();
				}
			});
		}
	}

	register uint64 result asm("rax");
	register __m128 arg_f_0 asm("xmm0") = f_storage[0];
	{
		register uint64 stack_remaining asm("rbx") = stack_size;
		register uint64 stack_beginning asm("rsi") = (uint64) stack_storage;

		register uint64 arg_0 asm("rcx") = i64_storage[0];
		register uint64 arg_1 asm("rdx") = i64_storage[1];
		register uint64 arg_2 asm("r8")  = i64_storage[2];
		register uint64 arg_3 asm("r9")  = i64_storage[3];

		register __m128 arg_f_1 asm("xmm1") = f_storage[1];
		register __m128 arg_f_2 asm("xmm2") = f_storage[2];
		register __m128 arg_f_3 asm("xmm3") = f_storage[3];

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
				"+r"(arg_1),   "+r"(arg_0),   "+r"(arg_2),   "+r"(arg_3),
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

	m.return_type().view(
		[&]<typename Type>(Type) {
			if constexpr(same_as<Type, class_file::v>) {
				stack.pop_back_until(stack_begin);
			}
			else if constexpr(
				same_as<Type, class_file::z> ||
				same_as<Type, class_file::b> ||
				same_as<Type, class_file::c> ||
				same_as<Type, class_file::s> ||
				same_as<Type, class_file::i>
			) {
				stack.pop_back_until(stack_begin);
				stack.emplace_back<int32>((int32)(uint32) result);
			}
			else if constexpr(same_as<Type, class_file::j>) {
				stack.pop_back_until(stack_begin);
				stack.emplace_back<int64>((int64) result);
			}
			else if constexpr(same_as<Type, class_file::f>) {
				stack.pop_back_until(stack_begin);
				stack.emplace_back<float>(arg_f_0[0]);
			}
			else if constexpr(same_as<Type, class_file::d>) {
				stack.pop_back_until(stack_begin);
				stack.emplace_back<double>(((__m128d) arg_f_0)[0]);
			}
			else if constexpr(
				same_as<Type, class_file::object> ||
				same_as<Type, class_file::array>
			) {
				// increment reference count before possible deletion on stack
				reference ref{ * (::object*) result };
				stack.pop_back_until(stack_begin);
				stack.emplace_back(move(ref));
			} else {
				abort();
			}
		}
	);
}

#endif