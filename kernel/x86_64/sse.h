#pragma once
#include <stdbool.h>
#include <size_t.h>

#define XCR0_X87_STATE  (1 << 0)
#define XCR0_ENABLE_SSE (1 << 1)
#define XCR0_ENABLE_AVX (1 << 2)

// *Enable FPU (Floating Point Unit)
inline void enable_fpu() {
    asm volatile("fninit");
}

static inline void _avx_save(uintptr_t addr) {
    asm volatile("xsave64 %0" : :  "m" (*(uint64_t*)addr), "d" (0xffffffff), "a" (0xffffffff) : "memory");
}

static inline void _avx_load(uintptr_t addr) {
    asm volatile("xrstor64 %0" : : "m" (*(uint64_t*)addr), "d" (0xffffffff), "a" (0xffffffff) : "memory");
}

static inline void _sse_save(uintptr_t addr) {
    asm volatile("fxsave %0" : : "m" (*(uint64_t*)addr));
}

static inline void _sse_load(uintptr_t addr) {
    asm volatile("fxrstor %0" : : "m" (*(uint64_t*)addr));
}

bool has_sse();
void enable_sse();
void init_sse();
void save_sse_context(uint8_t* context);
void load_sse_context(uint8_t* context);
size_t get_sse_context_size();
void set_initial_sse_context(uint8_t p[]);
