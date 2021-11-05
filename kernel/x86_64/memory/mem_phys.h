#pragma once
#include "libs/libc/stdint.h"

#define PHYSMEM_BLOCK_SIZE 0x1000

typedef enum {
    MEMORY_REGION_USABLE,
    MEMORY_REGION_RESERVED,
    MEMORY_REGION_KERNEL,
    MEMORY_REGION_FRAMEBUFFER
} memory_physical_region_type;

struct memory_physical_region {
    uint64_t base;
    uint64_t size;
    uint64_t limit;
    memory_physical_region_type type;
};

struct memory_physical {
    uint64_t total_memory;      // in bytes
    uint64_t usable_memory;     // in bytes
    uint64_t total_blocks;      // in bytes/block_size (blocks)
    uint64_t usable_blocks;     // in bytes/block_size (blocks)
    uint64_t used_blocks;

    struct memory_physical_region* regions;
    uint32_t* _map;
    uint64_t _map_size;
};

struct memory_physical pmm;

void init_pmm(struct memory_physical_region *entries, uint32_t size);
