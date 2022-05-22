#pragma once

#include <neutrino/macros.h>
#include <stdint.h>
#include "../interrupts.h"
#include "kernel/common/tasks/context.h"

struct __context {
    uintptr_t syscall_kstack; 
    uintptr_t syscall_ustack; 

    Registers regs;
    uint8_t simd[] aligned(64);
} aligned(64);

#define RFLAGS_INTERRUPT_ENABLE     (1<<9)
#define RFLAGS_RESERVED1_ONE        (1<<1)

void context_save(struct __context* context, const Registers* regs);
void context_load(struct __context* context, Registers* regs);
