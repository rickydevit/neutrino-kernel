#pragma once
#include <stdint.h>

// Syscall enum
typedef enum __neutrino_syscalls {
    NEUTRINO_TEST,
    NEUTRINO_SYSCALL_COUNT
} NeutrinoSyscall;

typedef enum __neutrino_syscall_result {
    SYSCALL_SUCCESS,
    SYSCALL_INVALID,
    SYSCALL_FAILURE
} SyscallResult;

#ifdef __kernel
#define SysCall(name) SyscallResult sys_##name
#else
#define SysCall(name) SyscallResult neutrino_##name

SysCall(syscall) (NeutrinoSyscall syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
#endif

// Syscall prototypes
SysCall(test)(uintptr_t* args);
