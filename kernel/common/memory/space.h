#pragma once

#include <size_t.h>
#include <stdint.h>

typedef struct __memory_range {
    uintptr_t base;
    size_t size;
} MemoryRange;

typedef struct __memory_range_node {
        MemoryRange range;
        struct __memory_range_node* next;
} MemoryRangeNode;

#ifdef __x86_64
#include "kernel/x86_64/memory/space.h"
#else
#error "Unsupported platform"
#endif

typedef struct __space Space;

Space* NewSpace();
void DestroySpace(Space* space);

void space_switch(Space* space);
uintptr_t space_map(Space* space, uintptr_t phys_addr, size_t size, uintptr_t virt_addr);
void space_unmap(Space* space, uintptr_t virt_addr);
