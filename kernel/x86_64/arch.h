#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <neutrino/time.h>

#define MEMV_OFFSET 0xffff800000000000
#define PERM_OFFSET 0xffff880000000000
#define MMIO_OFFSET 0xfffff00000000000
#define HEAP_OFFSET 0xfffffe0000000000
#define KERN_OFFSET 0xffffffff80000000

#define MemoryFunctions(name, offset) \
    static inline uintptr_t get_##name##_address(uintptr_t address) { return (uintptr_t)address + offset; }  \
    static inline uintptr_t get_r##name##_address(uintptr_t address) { return (uintptr_t)address - offset; }

MemoryFunctions(mem, MEMV_OFFSET)
MemoryFunctions(perm, PERM_OFFSET)
MemoryFunctions(mmio, MMIO_OFFSET)
MemoryFunctions(kern, KERN_OFFSET)

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

#define EFER_ENABLE_SYSCALL 0x1

// *Read a model specific register value and return it
// @param msr the model specific register to read
// @return the value of the register
static inline uint64_t read_msr(uint64_t msr) {
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

// *Write a value to a model specific register
// @param msr the model specific register to write
// @param value the value to write to the register
static inline void write_msr(uint64_t msr, uint64_t value) {
    asm volatile("wrmsr" : : "c"(msr), "a"(value & 0xFFFFFFFF), "d"(value >> 32));
}

void arch_idle();
Timestamp arch_now();
