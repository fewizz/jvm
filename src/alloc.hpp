#pragma once

#include <inttypes.h>

extern "C" void* __cdecl malloc(size_t size);
extern "C" void* __cdecl calloc(size_t size);
extern "C" void* __cdecl realloc(void *ptr, size_t new_size);
extern "C" void  __cdecl free(void* ptr);