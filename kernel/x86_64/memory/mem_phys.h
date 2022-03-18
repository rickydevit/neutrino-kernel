#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "size_t.h"

#define PHYSMEM_BLOCK_SIZE 0x1000
#define PHYSMEM_2MEGS      0x200000
#define PHYSMEM_MAP_BLOCKS_PER_UNIT (8*sizeof(*pmm._map))
#define PHYSMEM_MAP_BASE    (uintptr_t)pmm._map - (uintptr_t)pmm._map % PHYSMEM_BLOCK_SIZE
#define PHYSMEM_MAP_SIZE    (size_t)((pmm._map_size / PAGE_SIZE) + 1) * PHYSMEM_BLOCK_SIZE

typedef bool BlockState;
typedef int64_t BlockPosition;

#define BLOCKPOSITION_INVALID -1

#define Align(x) (x / PHYSMEM_BLOCK_SIZE)

typedef enum {
    MEMORY_REGION_USABLE,           // 0
    MEMORY_REGION_RESERVED,         // 1
    MEMORY_REGION_KERNEL,           // 2
    MEMORY_REGION_FRAMEBUFFER,      // 3
    MEMORY_REGION_ACPI_RSVD,        // 4
    MEMORY_REGION_ACPI_RCLM,        // 5

    MEMORY_REGION_INVALID           // 6
} memory_physical_region_type;

typedef struct __memory_physical_region {
    uint64_t base;
    uint64_t size;
    uint64_t limit;
    memory_physical_region_type type;
} MemoryPhysicalRegion;

struct memory_physical {
    uint64_t total_memory;      // in bytes
    uint64_t usable_memory;     // in bytes
    uint64_t total_blocks;      // in bytes/block_size (blocks)
    uint64_t usable_blocks;     // in bytes/block_size (blocks)
    uint64_t used_blocks;

    MemoryPhysicalRegion* regions;
    uint32_t regions_count; 
    uint32_t* _map;
    uint64_t _map_size;
};

struct memory_physical pmm;

void init_pmm(MemoryPhysicalRegion* entries, uint32_t size);
uintptr_t pmm_alloc(); 
uintptr_t pmm_alloc_zero(); 
void pmm_free(uintptr_t addr);
uintptr_t pmm_alloc_series(size_t size); 
void pmm_free_series(uintptr_t addr, size_t size); 
MemoryPhysicalRegion pmm_get_region_by_type(memory_physical_region_type type);
