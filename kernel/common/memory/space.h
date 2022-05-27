#pragma once

#include <size_t.h>
#include <stdint.h>

#ifdef __x86_64
#include "kernel/x86_64/memory/space.h"
#else
#error "Unsupported platform"
#endif

typedef enum __mapping_flags {
    MAP_USER = 0b1,
    MAP_WRITABLE = 0b10,
} MappingFlags;

typedef struct __space Space;

Space* NewSpace();
void DestroySpace(Space* space);

void space_switch(Space* space);
void space_map(Space* space, uintptr_t phys_addr, uintptr_t virt_addr, size_t size, MappingFlags flags);
void space_unmap(Space* space, uintptr_t virt_addr);
