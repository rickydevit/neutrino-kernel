#pragma once
#include <stdint.h>
#include <neutrino/time.h>

// Syscall enum
typedef enum __neutrino_syscalls {
    NEUTRINO_TEST,
    NEUTRINO_KILL_TASK,
    NEUTRINO_NOW,
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

// === Syscall argument types ===

typedef struct __sc_now_args {
    Timestamp timestamp;
} SCNowArgs;

// === Syscall prototypes ===

SysCall(test)(uintptr_t* args);

// Destroy the current task. This is a TRAP syscall, control is lost and returned to kernel
// @param args unused argument
// @return SYSCALL_FAILURE on call failure (scheduler fails to terminate task)
SysCall(destroy_task)(uintptr_t* args);

// Return the current timestamp. Note this is not the same as UNIX timestamp
// @param timestamp the current timestamp 
// @return SYSCALL_SUCCESS on success
SysCall(now)(SCNowArgs* args);
