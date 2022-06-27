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
	inline stack_entry call(span<stack_entry, uint16> args);

	auto name() { return name_; }

	~native_function() {
		default_allocator{}.deallocate((uint8*) name_.data(), name_.size());
	}
};

#include "../jni/environment.hpp"

template<typename ReturnType>
inline stack_entry native_function::call(span<stack_entry, uint16> args) {
	// TODO use asm instead
	if(args.size() == 0) {
		if constexpr(same_as<ReturnType, jvoid>) {
			((void(*)(jni_environment* env)) ptr_ )(nullptr);
			return jvoid{};
		}
		if constexpr(same_as<ReturnType, jbool>) {
			jbool res = ((jbool(*)(jni_environment* env)) ptr_ )(nullptr);
			return jint{ int32(res.value) };
		}
	} else if (args.size() == 1) {
		if(args[0].is<reference>()) {
			if constexpr(same_as<ReturnType, jvoid>) {
				((void* (*)(jni_environment* env, object*)) ptr_ )
				(nullptr, args[0].get<reference>().object_ptr());
				return jvoid{};
			}
			if constexpr(same_as<ReturnType, jbool>) {
				jbool res =
					((jbool (*)(jni_environment* env, object*)) ptr_ )
					(nullptr, args[0].get<reference>().object_ptr());
				return jint{ int32(res.value) };
			}
			if constexpr(same_as<ReturnType, jint>) {
				return
					((jint (*)(jni_environment* env, object*)) ptr_ )
					(nullptr, args[0].get<reference>().object_ptr());
			}
			if constexpr(same_as<ReturnType, reference>) {
				auto obj_ptr =
					((object* (*)(jni_environment* env, object*)) ptr_ )
					(nullptr, args[0].get<reference>().object_ptr());
				return reference{ *obj_ptr };
			}
		}
	} else if (args.size() == 2) {
		if(args[0].is<reference>()) {
			if(args[1].is<reference>()) {
				if constexpr(same_as<ReturnType, jint>) {
					return ((jint (*)(jni_environment*, object*, object*)) ptr_)
					(
						nullptr,
						args[0].get<reference>().object_ptr(),
						args[1].get<reference>().object_ptr()
					);
				}
			}
		}
	} else if (args.size() == 5) {
		if(args[0].is<reference>()) {
			if(args[1].is<jint>()) {
				if(args[2].is<reference>()) {
					if(args[3].is<jint>()) {
						if(args[4].is<jint>()) {
							if constexpr(same_as<ReturnType, jvoid>) {
								((void (*)(jni_environment*,
									object*, jint, object*, jint, jint
								)) ptr_)
								(
									nullptr,
									args[0].get<reference>().object_ptr(),
									args[1].get<jint>(),
									args[2].get<reference>().object_ptr(),
									args[3].get<jint>(),
									args[4].get<jint>()
								);
								return jvoid{};
							}
						}
					}
				}
			}
		}
	}

	fputs("couldn't call native function", stderr);
	abort();
}