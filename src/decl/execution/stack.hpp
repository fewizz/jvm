#pragma once

#include "./stack_entry.hpp"

#include <list.hpp>

#include <posix/memory.hpp>

/*template<template<typename> typename StorageRange>
struct stack : list<StorageRange<storage<stack_entry>>> {
	using base_type = list<StorageRange<stack_entry>>;
	using base_type::base_type;
};*/