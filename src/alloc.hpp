#pragma once

#include <core/integer.hpp>

extern "C" void* __cdecl malloc(nuint size);
extern "C" void* __cdecl calloc(nuint size);
extern "C" void* __cdecl realloc(void *ptr, nuint new_size);
extern "C" void  __cdecl free(void* ptr);

struct default_allocator {

	uint8* allocate(auto size) {
		return (uint8*) malloc(size);
	}

	void deallocate(uint8* ptr, auto) {
		free(ptr);
	}

};