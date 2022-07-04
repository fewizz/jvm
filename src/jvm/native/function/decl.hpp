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
public:

	template<typename Name>
	native_function(void* ptr, Name name) :
		ptr_{ ptr }
	{
		span s {
			(char*) default_allocator{}.allocate(name.size()), name.size()
		};
		copy{ name }.to(s);
		name_ = { s.data(), s.size() };
	}

	native_function(const native_function&) = delete;
	native_function& operator = (const native_function&) = delete;

	native_function(native_function&& other) :
		ptr_{ exchange(other.ptr_, nullptr) },
		name_{ move(other.name_) }
	{}

	native_function& operator = (native_function&& other) {
		ptr_ = exchange(other.ptr_, nullptr);
		name_ = move(other.name_);
		return *this;
	}

	operator void* () { return ptr_; }

	template<typename ReturnType>
	stack_entry call(span<stack_entry, uint16> args);

	auto name() { return name_; }

	~native_function() {
		default_allocator{}.deallocate((uint8*) name_.data(), name_.size());
	}
};

#include "../jni/environment.hpp"

template<typename ReturnType, typename... Args>
static inline stack_entry call_native_function1(void* ptr, Args... args) {
	if constexpr(same_as<ReturnType, jvoid>) {
		((void(*)(jni_environment*, Args...)) ptr)(nullptr, args...);
		return jvoid{};
	}
	if constexpr(same_as<ReturnType, jbool>) {
		return jint { (int32)
			((bool(*)(jni_environment*, Args...)) ptr)
			(nullptr, args...)
		};
	}
	if constexpr(same_as<ReturnType, jint>) {
		return jint {
			((int(*)(jni_environment*, Args...)) ptr)
			(nullptr, args...)
		};
	}
	if constexpr(same_as<ReturnType, jfloat>) {
		return jfloat {
			((float(*)(jni_environment*, Args...)) ptr)
			(nullptr, args...)
		};
	}
	if constexpr(same_as<ReturnType, jlong>) {
		return jlong {
			((int64(*)(jni_environment*, Args...)) ptr)
			(nullptr, args...)
		};
	}
	if constexpr(same_as<ReturnType, jdouble>) {
		return jdouble {
			((double(*)(jni_environment*, Args...)) ptr)
			(nullptr, args...)
		};
	}
	if constexpr(same_as<ReturnType, reference>) {
		return reference { *
			((object*(*)(jni_environment*, Args...)) ptr)
			(nullptr, args...)
		};
	}
	fputs("unknown return type", stderr); abort();
}

template<typename ReturnType, typename... Args>
static inline stack_entry call_native_function0(
	void* ptr,
	span<stack_entry, uint16> rem,
	Args... args
) {
	if(rem.size() == 0) {
		return call_native_function1<ReturnType>(ptr, args...);
	}
	if constexpr(sizeof...(Args) > 2) {
		fputs("too many arguments", stderr); abort();
	} else {
	span<stack_entry, uint16> next_rem{
		rem.begin() + 1, (uint16) (rem.size() - 1)
	};
	if(rem[0].is<jint>()) {
		return call_native_function0<ReturnType>(
			ptr, next_rem, args..., (int32) rem[0].get<jint>()
		);
	}
	if(rem[0].is<jfloat>()) {
		return call_native_function0<ReturnType>(
			ptr, next_rem, args..., (float) rem[0].get<jfloat>()
		);
	}
	if(rem[0].is<jlong>()) {
		return call_native_function0<ReturnType>(
			ptr, next_rem, args..., (int64) rem[0].get<jlong>()
		);
	}
	if(rem[0].is<jdouble>()) {
		return call_native_function0<ReturnType>(
			ptr, next_rem, args..., (double) rem[0].get<jdouble>()
		);
	}
	if(rem[0].is<reference>()) {
		return call_native_function0<ReturnType>(
			ptr, next_rem, args..., rem[0].get<reference>().object_ptr()
		);
	}
	fputs("unknown arg type", stderr); abort();
	}
}

template<typename ReturnType>
inline stack_entry native_function::call(span<stack_entry, uint16> args) {
	return call_native_function0<ReturnType>(ptr_, args);
}