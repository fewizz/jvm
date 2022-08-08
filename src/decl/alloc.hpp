#pragma once

#include <memory_span.hpp>

extern "C" void* __cdecl malloc(nuint size);
extern "C" void* __cdecl calloc(nuint num, nuint size);
extern "C" void* __cdecl realloc(void *ptr, nuint new_size);
extern "C" void  __cdecl free(void* ptr);

inline memory_span allocate(auto size) {
	return { malloc(size), size };
}

inline memory_span allocate_zeroed(auto size) {
	return { (uint8*) calloc(size, 1), size };
}

inline void deallocate(memory_span memory_span) {
	free(memory_span.elements_ptr());
}