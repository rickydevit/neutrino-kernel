#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <size_t.h>
#include <ipc/ipc.h>
#include <neutrino/time.h>
#include <neutrino/macros.h>
#include <neutrino/file/file.h>

#define FOREACH_SYSCALL(c) \
    c(NEUTRINO_LOG) \
    c(NEUTRINO_KILL_TASK) \
    c(NEUTRINO_NOW) \
    c(NEUTRINO_ALLOC) \
    c(NEUTRINO_FREE) \
    c(NEUTRINO_IPC) \
    c(NEUTRINO_FILE) \

// Syscall enum
typedef enum __neutrino_syscalls {
    FOREACH_SYSCALL(GenerateEnum)
    NEUTRINO_SYSCALL_COUNT
} NeutrinoSyscall;

extern const char* syscall_names[];

typedef enum __neutrino_syscall_result {
    SYSCALL_SUCCESS,
    SYSCALL_INVALID,
    SYSCALL_FAILURE,
    SYSCALL_UNAUTHORIZED
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

typedef struct __sc_free_args {
    uintptr_t pointer;
    size_t size;
} SCFreeArgs;

typedef struct __sc_ipc_args {
    const char* agent_name;
    IpcType type;
    uintptr_t payload;
    size_t size;
} SCIpcArgs;

typedef struct __sc_file_args {
    int64_t id;
    const char* file_path;
    FileOperation operation;
} SCFileArgs;

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

// Free a user-heap memory area previously allocated
// @param pointer IN the address to be freed
// @param size IN the size of the memory area
// @return SYSCALL_SUCCESS on success; SYSCALL_INVALID if size = 0 or pointer is nullptr; SYSCALL_FAILURE if heap manager fails
SysCall(free)(SCFreeArgs* args);

// Send an IPC message (between different processes)
// @param type IN the type of the IPC call (SEND, RECEIVE, BROADCAST)
// @param data IN/OUT the data to be sent/received. This is an output field when type is RECEIVE, input otherwise
// @param size IN/OUT the size of the sent/received data. This is an output field when type is RECEIVE, input otherwise 
// @return SYSCALL_SUCCESS on success; SYSCALL_UNAUTHORIZED if task channel does not allow IPCs; SYSCALL_FAILURE if IPC fails
SysCall(ipc)(SCIpcArgs* args);

// Request a file descriptor for a given path
// @param path IN the path of the file to request
// @param operation IN the operation to be performed on the file 
// @param descriptor IN/OUT the file descriptor; -1 if invalid
// @return SYSCALL_SUCCESS on success; SYSCALL_INVALID if the file does not exist; SYSCALL_FAILURE if the file cannot be opened
SysCall(file)(SCFileArgs* args);
