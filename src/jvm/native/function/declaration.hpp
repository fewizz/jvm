#pragma once

#include "../../execute/stack_entry.hpp"
#include "../../../alloc.hpp"
#include <core/c_string.hpp>
#include <core/range.hpp>
#include <core/copy.hpp>
#include <core/span.hpp>

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

	inline stack_entry call(span<stack_entry, uint16> args);

	auto name() { return name_; }

	~native_function() {
		default_allocator{}.deallocate((uint8*) name_.data(), name_.size());
	}
};

#include "../jni/environment.hpp"

inline stack_entry native_function::call(span<stack_entry, uint16> args) {
	// TODO use asm instead
	if(args.size() == 0) {
		((void(*)(jni_environment* env)) ptr_ )(nullptr);
		return jvoid{};
	}
	else {
		fputs("couldn't call native function", stderr);
		abort();
	}
}