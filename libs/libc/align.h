#pragma once
#include "size_t.h"
#include "stdint.h"

static inline void* align(size_t align, size_t size, void* ptr, size_t space) {
	intptr_t int_ptr = (intptr_t)ptr, offset = (-int_ptr) & (align - 1);
	if ((size + offset) > space) return nullptr;

	intptr_t aligned = int_ptr + offset;
	return (void*)aligned;
}
