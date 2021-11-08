#pragma once
#include "libs/libc/stdint.h"
#include "libs/libc/size_t.h"

#define PHYSMEM_BLOCK_SIZE 0x1000

typedef enum {
    MEMORY_REGION_USABLE,           // 0
    MEMORY_REGION_RESERVED,         // 1
    MEMORY_REGION_KERNEL,           // 2
    MEMORY_REGION_FRAMEBUFFER,      // 3
    MEMORY_REGION_ACPI,             // 4

    MEMORY_REGION_INVALID           // 5
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
    uint32_t regions_count; 
    uint32_t* _map;
    uint64_t _map_size;
};

struct memory_physical pmm;

void init_pmm(struct memory_physical_region *entries, uint32_t size);
uintptr_t pmm_alloc(); 
void pmm_free(uintptr_t addr);
uintptr_t pmm_alloc_series(size_t size); 
void pmm_free_series(uintptr_t addr, size_t size); 
struct memory_physical_region pmm_get_region_by_type(memory_physical_region_type type);
