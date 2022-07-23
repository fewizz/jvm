#pragma once

#include "execution/stack_entry.hpp"
#include "alloc.hpp"
#include "abort.hpp"

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

	stack_entry call(span<stack_entry, uint16> args);

	auto name() { return name_; }
};

#include "../jni/environment.hpp"
#include <core/bit_cast.hpp>
#include <class/file/descriptor/reader.hpp>