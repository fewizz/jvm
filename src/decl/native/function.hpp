#pragma once

#include "execution/stack_entry.hpp"
#include "alloc.hpp"
#include "abort.hpp"

#include <c_string.hpp>
#include <range.hpp>
#include <span.hpp>
#include <exchange.hpp>

#include <stdio.h>

struct native_function {
private:
	void* ptr_;
	memory_span name_;
	memory_span desc_;
public:

	template<basic_range Name, basic_range Descriptor>
	native_function(void* ptr, Name&& name, Descriptor&& desc) :
		ptr_{ ptr },
		name_{ allocate(name.size()) },
		desc_{ allocate(desc.size()) }
	{
		range{ name }.copy_to(name_);
		range{ desc }.copy_to(desc_);
	}

	~native_function() {
		deallocate(name_);
		deallocate(desc_);
	}

	native_function(const native_function&) = delete;
	native_function& operator = (const native_function&) = delete;

	operator void* () { return ptr_; }

	stack_entry call(span<stack_entry, uint16> args);

	auto name() { return name_; }
};

#include "native/jni/environment.hpp"
#include <bit_cast.hpp>
#include <class_file/descriptor/reader.hpp>