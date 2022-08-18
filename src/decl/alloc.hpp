#pragma once

#include "./abort.hpp"

#include <integer.hpp>
#include <memory_span.hpp>
#include <optional.hpp>

extern "C" void* __cdecl malloc(nuint size);
extern "C" void* __cdecl calloc(nuint num, nuint size);
extern "C" void* __cdecl realloc(void *ptr, nuint new_size);
extern "C" void  __cdecl free(void* ptr);

inline optional<memory_span> try_allocate(nuint size) {
	uint8* ptr = (uint8*) malloc(size);
	if(ptr == nullptr) { return {}; }
	return memory_span{ ptr, size };
}

template<typename Type>
inline optional<memory_span> try_allocate_for(nuint count) {
	return try_allocate(sizeof(Type) * count);
}

inline memory_span allocate(nuint size) {
	return try_allocate(size).if_no_value(abort).value();
}

template<typename Type>
inline memory_span allocate_for(nuint count) {
	return allocate(sizeof(Type) * count);
}

template<typename Type>
inline optional<memory_span> try_allocate_zeroed_for(nuint count) {
	uint8* ptr = (uint8*) calloc(count, sizeof(Type));
	if(ptr == nullptr) {
		return {};
	}
	return memory_span{ ptr, /* not always true */ sizeof(Type) * count };
}

template<typename Type>
inline memory_span allocate_zeroed_for(nuint count) {
	return try_allocate_zeroed_for<Type>(count).if_no_value(abort).value();
}

inline void deallocate(memory_span memory_span) {
	free(memory_span.elements_ptr());
}