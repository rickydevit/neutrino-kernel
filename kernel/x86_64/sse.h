#pragma once
#include "libs/libc/stdbool.h"

#define XCR0_X87_STATE  (1 << 0)
#define XCR0_ENABLE_SSE (1 << 1)
#define XCR0_ENABLE_AVX (1 << 2)

// *Enable FPU (Floating Point Unit)
inline void enable_fpu() {
    asm("fninit");
}

inline void _avx_save(uintptr_t addr) {
    __asm__("xsave64 %0" : :  "m" (*(uint64_t*)addr), "d" (0xffffffff), "a" (0xffffffff));
}

inline void _avx_load(uintptr_t addr) {
    __asm__("xrstor64 %0" : : "m" (*(uint64_t*)addr), "d" (0xffffffff), "a" (0xffffffff));
}

inline void _sse_save(uintptr_t addr) {
    __asm__("fxsave %0" : : "m" (*(uint64_t*)addr));
}

inline void _sse_load(uintptr_t addr) {
    __asm__("fxrstor %0" : : "m" (*(uint64_t*)addr));
}

bool has_sse();
void enable_sse();
void init_sse();
