#include "syscall.h"
#include <stdint.h>

// === PRIVATE FUNCTIONS ========================

#ifdef __x86_64

SyscallResult neutrino_syscall(NeutrinoSyscall syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    uint64_t res;

    register uint64_t r8 asm("r8") = arg4;
    register uint64_t r9 asm("r9") = arg5;

    asm volatile("syscall" 
                : "=a"(res) 
                : "a"(syscall_id), "b"(arg1), "d"(arg2), "S"(arg3), "r"(r8), "r"(r9)
                : "memory", "r11", "rcx");

    return (SyscallResult)res;
}

#else
#error "Unsupported platform"
#endif

// === PUBLIC FUNCTIONS =========================

SyscallResult neutrino_test(uintptr_t* args) {
    return neutrino_syscall(NEUTRINO_TEST, (uintptr_t)args, 0, 0, 0, 0);
}
