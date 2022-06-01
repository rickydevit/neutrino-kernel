#pragma once
#include <stdint.h>
#include <size_t.h>

typedef struct __memory_range {
    uintptr_t base;
    size_t size;
} MemoryRange;

#define RangeEnd(range)  (range.base + range.size)

typedef struct __memory_range_node {
        MemoryRange range;
        struct __memory_range_node* next;
} MemoryRangeNode;

typedef struct __virtual_mapping {
    MemoryRange physical;
    uintptr_t virtual_base;
} VirtualMapping;

void memory_copy(uint8_t *source, uint8_t *dest, int nbytes);
void memory_set(uint8_t *dest, uint8_t val, uint32_t len);

VirtualMapping memory_allocate(size_t size);
void memory_free(VirtualMapping mapping);
