#include "syscall.h"
#include "arch.h"
#include "kservice.h"
#include "tasks/task.h"
#include "tasks/channel.h"
#include "memory/space.h"
#include "tasks/scheduler.h"
#include "handle/handle.h"
#include <neutrino/syscall.h>
#include <linkedlist.h>
#include <ipc/ipc.h>
#include <stdint.h>

// === PRIVATE FUNCTIONS ========================

SyscallResult sys_log(SCLogArgs* args) {
    if (args->msg == nullptr) return SYSCALL_INVALID;

    ks.log(args->msg);
    return SYSCALL_SUCCESS;
}

SyscallResult sys_destroy_task(uintptr_t* args) {
    sched_terminate();
    return SYSCALL_FAILURE; // return failure on sched_terminate return 
}

SyscallResult sys_now(SCNowArgs* args) {
    *args = (SCNowArgs){
        .timestamp = arch_now(),
    };

    return SYSCALL_SUCCESS;
}

SyscallResult sys_alloc(SCAllocArgs* args) {
    if (args->size == 0) return SYSCALL_INVALID;
    args->pointer = vmm_allocate_heap(args->size, args->user);

    if (args->pointer == nullptr) return SYSCALL_FAILURE;
    return SYSCALL_SUCCESS;
}

SyscallResult sys_free(SCFreeArgs* args) {
    if (args->size == 0 || args->pointer == nullptr) return SYSCALL_INVALID;
    if (vmm_free_memory(0, args->pointer, args->size) == true) return SYSCALL_SUCCESS;

    return SYSCALL_FAILURE;
}

SyscallResult sys_ipc(SCIpcArgs* args) {
    if (args->type > IPC_BROADCAST) return SYSCALL_INVALID;

    if (args->type == IPC_SEND || args->type == IPC_BROADCAST) {
        if (args->payload == nullptr || args->size == 0) return SYSCALL_INVALID;

        Channel* sender = get_current_task()->channel;
        Channel* dest = channel_find_by_agent_name(args->agent_name);
        if (!sender || !dest) return SYSCALL_FAILURE;

        Package* pack = NewPackage(&sender->agent, args->payload, args->size);

        switch (channel_transmit(sender, dest, pack)) {
            case CHANNEL_TRANSMIT_UNAUTHORIZED_SENDER:
            case CHANNEL_TRANSMIT_UNAUTHORIZED_RECIPIENT:
                return SYSCALL_UNAUTHORIZED;
            case CHANNEL_TRANSMIT_SUCCESS:
                return SYSCALL_SUCCESS;
            default:
                return SYSCALL_FAILURE;
        }
    }

    if (args->type == IPC_RECEIVE || args->type == IPC_PEEK) {
        Channel* recv = get_current_task()->channel;
        Package* pack = nullptr;

        if (!recv) return SYSCALL_FAILURE;

        if (args->type == IPC_RECEIVE) {
            ChannelReceiveResult res = channel_receive(recv, &pack);
            if (pack && res == CHANNEL_RECEIVE_SUCCESS) {
                memory_copy((uint8_t*)pack->buffer, (uint8_t*)args->payload, pack->size);
                args->size = pack->size;
                kfree(pack);
            }

            switch (res) {
                case CHANNEL_RECEIVE_UNAUTHORIZED_RECEIVER:
                    return SYSCALL_UNAUTHORIZED;
                case CHANNEL_RECEIVE_BUFFER_EMPTY:
                case CHANNEL_RECEIVE_SUCCESS:
                    return SYSCALL_SUCCESS;
                default:
                    return SYSCALL_FAILURE;
            }
        } else {
            ChannelPeekResult res = channel_peek(recv, &pack);
            if (pack && res == CHANNEL_PEEK_SUCCESS) {
                args->size = pack->size;
            }

            switch (res) {
                case CHANNEL_PEEK_UNAUTHORIZED_RECEIVER:
                    return SYSCALL_UNAUTHORIZED;
                case CHANNEL_PEEK_BUFFER_EMPTY:
                case CHANNEL_PEEK_SUCCESS:
                    return SYSCALL_SUCCESS;
                default:
                    return SYSCALL_FAILURE;
            }
        }
        
    }

    return SYSCALL_FAILURE;
}

SyscallResult sys_file(SCFileArgs* args) {
    if (args->operation > FILE_OPEN) return SYSCALL_FAILURE;
    if (args->file_path == nullptr) return SYSCALL_INVALID;

    args->id = -1;

    Task* task = get_current_task();
    HandleRequest request = {.type = HANDLE_TYPE_FILE, .path = args->file_path};

    switch (args->operation) {
        case FILE_OPEN:
            if (handle_acquire(task, request) == HANDLE_RESULT_SUCCESS) {
                args->id = list_get_size(task->handles)-1;
                return SYSCALL_SUCCESS;
            } else {
                return SYSCALL_FAILURE;
            }
            
        default:
            return SYSCALL_INVALID;
    }
}

// === PUBLIC FUNCTIONS =========================

typedef SyscallResult SyscallFn();

SyscallFn* syscalls[NEUTRINO_SYSCALL_COUNT] = {
    [NEUTRINO_LOG] = sys_log,
    [NEUTRINO_KILL_TASK] = sys_destroy_task,
    [NEUTRINO_NOW] = sys_now,
    [NEUTRINO_ALLOC] = sys_alloc,
    [NEUTRINO_FREE] = sys_free,
    [NEUTRINO_IPC] = sys_ipc,
    [NEUTRINO_FILE] = sys_file,
};

SyscallResult syscall_execute(NeutrinoSyscall syscall_id, uintptr_t* args) {
    if (syscall_id >= NEUTRINO_SYSCALL_COUNT)
        return SYSCALL_INVALID;
    
    task_start_syscall();

    SyscallResult result = syscalls[syscall_id](args);

    const Task* cur = get_current_task();
    ks.log("Syscall %c in task #%u (%c) with result %u", syscall_names[syscall_id], cur->pid, cur->name, result);

    task_end_syscall();

    return result;
}
