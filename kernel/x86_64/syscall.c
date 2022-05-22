#include "syscall.h"
#include "arch.h"
#include "interrupts.h"
#include "kernel/common/syscall.h"
#include <stdint.h>
#include <neutrino/macros.h>
#include <neutrino/syscall.h>

extern void _syscall(void);

// === PRIVATE FUNCTIONS ========================

// === PUBLIC FUNCTIONS =========================

void init_syscall() {
    write_msr(EFER, read_msr(EFER) | EFER_ENABLE_SYSCALL);

    // STAR: 63 -------- 47 -------- 31 -------- 15 -------- 0
    //          user seg    kern seg     32bit EIP (unused)
    //            0x28        0x18
    write_msr(STAR, ((uint64_t)(0x18) << 32) | ((uint64_t)(0x28) << 48));

    // LSTAR: 64bit RIP entry point
    write_msr(LSTAR, ((uintptr_t)_syscall));

    // FLAG MASK
    write_msr(SYSCALL_FLAG_MASK, 0xfffffffe);
}

void syscall_set_gs(uintptr_t addr) {
    write_msr(GS_BASE, addr);
    write_msr(KERN_GS_BASE, addr);
}

uint64_t unoptimized syscall_handler(Registers* regs) {
    return syscall_execute((NeutrinoSyscall)regs->rax, (uintptr_t*)regs->rbx);
}
