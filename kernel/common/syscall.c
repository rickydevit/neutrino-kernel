#include "syscall.h"
#include "kservice.h"
#include "tasks/task.h"
#include <neutrino/syscall.h>
#include <stdint.h>

// === PRIVATE FUNCTIONS ========================

SyscallResult sys_test(uintptr_t* args) {
    ks.dbg("Hello syscall world!");
    return SYSCALL_SUCCESS;
}

// === PUBLIC FUNCTIONS =========================

typedef SyscallResult SyscallFn();

SyscallFn* syscalls[NEUTRINO_SYSCALL_COUNT] = {
    [NEUTRINO_TEST] = sys_test
};

SyscallResult syscall_execute(NeutrinoSyscall syscall_id, uintptr_t* args) {
    if (syscall_id >= NEUTRINO_SYSCALL_COUNT)
        return SYSCALL_INVALID;
    
    task_start_syscall();

    SyscallResult result = syscalls[syscall_id](args);

    const Task* cur = get_current_task();
    ks.log("Syscall: #%u in task #%u (%c)", syscall_id, cur->pid, cur->name);

    task_end_syscall();

    return result;
}
