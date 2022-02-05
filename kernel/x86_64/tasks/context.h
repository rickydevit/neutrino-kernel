#pragma once

#include <neutrino/macros.h>
#include <stdint.h>
#include "../interrupts.h"
#include "kernel/common/tasks/context.h"

struct __context {
    // uintptr_t syscall_kstack; // todo syscalls
    // uintptr_t syscall_ustack; // todo syscalls

    Registers regs;
    uint8_t* simd aligned(64);
} packed;

#define RFLAGS_INTERRUPT_ENABLE     (1<<9)
#define RFLAGS_RESERVED1_ONE        (1<<1)

void context_save(struct __context* context, const Registers* regs);
void context_load(struct __context* context, Registers* regs);
