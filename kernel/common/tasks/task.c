#include "task.h"
#include "context.h"
#include "channel.h"
#include "cpu.h"
#include "../kservice.h"
#include "../memory/memory.h"
#include "../memory/space.h"
#include <liballoc.h>
#include <string.h>
#include <stdbool.h>
#include <neutrino/macros.h>

static uint32_t _global_pid = 0;

// === PRIVATE FUNCTIONS ========================

void task_set_name(Task* task, char* name) {
    memory_set((uint8_t*)task->name, 0, TASK_NAME_MAX);

    if (strlen(name) > TASK_NAME_MAX) {
        ks.warn("Task name is longer than the maximum allowed (%i). The name will be truncated.", TASK_NAME_MAX);
        memory_copy((uint8_t*)name, (uint8_t*)task->name, TASK_NAME_MAX);
        task->name[TASK_NAME_MAX] = '\0';
    } else {
        strcpy(name, task->name);
    }
}

// === PUBLIC FUNCTIONS =========================

Task* unoptimized NewTask(char* name, bool user) {
    Task* task = (Task*)kmalloc(sizeof(Task));

    task_set_name(task, name);

    task->pid = _global_pid++;
    task->status = TASK_EMBRYO;
    task->user = user;
    task->lock = NewLock;
    task->space = NewSpace();
    task->context = NewContext();
    task->channel = NewChannel(CHANNEL_CAN_RECEIVE | CHANNEL_CAN_SEND, name);

    task->in_io = false;
    task->in_syscall = false;

    task_set_stack(task, user);
    return task;
}

Task* unoptimized NewIdleTask(uintptr_t entry_point) {
    Task* idle = NewTask("idle", false);
    context_init(idle->context, entry_point, PROCESS_STACK_BASE + PROCESS_STACK_SIZE, PROCESS_STACK_BASE, (ContextFlags){0});

    idle->status = TASK_NEW;

    return idle;
}

void DestroyTask(Task* task) {
    DestroySpace(task->space);
    DestroyContext(task->context);
    DestroyChannel(task->channel);
    kfree(task);
}

Task* get_current_task() {
    return get_current_cpu()->tasks.current;
}

void task_start_syscall() {
    get_current_task()->in_syscall = true;
}

void task_end_syscall() {
    get_current_task()->in_syscall = false;
}
