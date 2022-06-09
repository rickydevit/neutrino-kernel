#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <size_t.h>
#include <neutrino/time.h>

// Syscall enum
typedef enum __neutrino_syscalls {
    NEUTRINO_LOG,
    NEUTRINO_KILL_TASK,
    NEUTRINO_NOW,
    NEUTRINO_ALLOC,
    NEUTRINO_FREE,
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

typedef struct __sc_log_args {
    char* msg;
} SCLogArgs;

typedef struct __sc_now_args {
    Timestamp timestamp;
} SCNowArgs;

typedef struct __sc_alloc_args {
    size_t size;
    uintptr_t pointer;
    bool user;
} SCAllocArgs;

// === Syscall prototypes ===

// Log a string message to the debug serial output
// @param msg IN the message to log
// @return SYSCALL_SUCCESS on success; SYSCALL_INVALID if msg is nullptr
SysCall(log)(SCLogArgs* args);

// Destroy the current task. This is a TRAP syscall, control is lost and returned to kernel
// @param args unused argument
// @return SYSCALL_FAILURE on call failure (scheduler fails to terminate task)
SysCall(destroy_task)(uintptr_t* args);

// Return the current timestamp. Note this is not the same as UNIX timestamp
// @param timestamp OUT the current timestamp 
// @return SYSCALL_SUCCESS on success
SysCall(now)(SCNowArgs* args);

// Return a user-heap memory pointer of the given size
// @param size IN the size of the memory area to allocate
// @param pointer OUT the pointer to the allocated area
// @return SYSCALL_SUCCESS on success; SYSCALL_INVALID if size = 0; SYSCALL_FAILURE if pointer is nullptr
SysCall(alloc)(SCAllocArgs* args);
