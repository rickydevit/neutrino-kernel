#pragma once
#include "libs/libc/stdint.h"

#define MEMV_OFFSET 0xffff800000000000
#define KERN_OFFSET 0xffffffff80000000

inline uintptr_t get_mem_address(uintptr_t addr) { return addr + MEMV_OFFSET; }
inline uintptr_t get_rmem_address(uintptr_t addr) { return addr - MEMV_OFFSET; }
