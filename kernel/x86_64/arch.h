#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MEMV_OFFSET 0xffff800000000000
#define MMIO_OFFSET 0xfffff00000000000
#define HEAP_OFFSET 0xfffffe0000000000
#define KERN_OFFSET 0xffffffff80000000

static inline uintptr_t get_mem_address(uintptr_t addr) { return (uintptr_t)addr + MEMV_OFFSET; }
static inline uintptr_t get_rmem_address(uintptr_t addr) { return (uintptr_t)addr - MEMV_OFFSET; }
static inline uintptr_t get_kern_address(uintptr_t addr) { return (uintptr_t)addr + KERN_OFFSET; }
static inline uintptr_t get_rkern_address(uintptr_t addr) { return (uintptr_t)addr - KERN_OFFSET; }
static inline uintptr_t get_mmio_address(uintptr_t addr) { return (uintptr_t)addr + MMIO_OFFSET; }
static inline uintptr_t get_rmmio_address(uintptr_t addr) { return (uintptr_t)addr - MMIO_OFFSET; }

enum MSR_REGISTERS {
    APIC =              0x1B,
    EFER =              0xC0000080,
    STAR =              0xC0000081,
    LSTAR =             0xC0000082,
    COMPAT_STAR =       0xC0000083,
    SYSCALL_FLAG_MASK = 0xC0000084,
    FS_BASE =           0xC0000100,
    GS_BASE =           0xC0000101,
    KERN_GS_BASE =      0xc0000102,
};

// *Read a model specific register value and return it
// @param msr the model specific register to read
// @return the value of the register
inline uint64_t read_msr(uint64_t msr) {
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

// *Write a value to a model specific register
// @param msr the model specific register to write
// @param value the value to write to the register
inline void write_msr(uint64_t msr, uint64_t value) {
    asm volatile("wrmsr" : : "c"(msr), "a"(value & 0xFFFFFFFF), "d"(value >> 32));
}

static inline void arch_idle() {
    while (true) asm volatile ("hlt");
}
