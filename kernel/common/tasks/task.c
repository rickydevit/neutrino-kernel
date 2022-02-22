#include "task.h"
#include "context.h"
#include "../kservice.h"
#include "../memory/memory.h"
#include "../memory/space.h"
#include <liballoc.h>
#include <string.h>

// === PRIVATE FUNCTIONS ========================

void task_set_stack(Task* task) {
    task->stack = (uintptr_t)kmalloc(PROCESS_STACK_SIZE);
    task->context->regs.rsp = task->stack;
    space_map(task->space, task->stack, PROCESS_STACK_SIZE, PROCESS_STACK_BASE);
}

// === PUBLIC FUNCTIONS =========================

Task* NewTask(const char* name, bool user) {
    Task* task = (Task*)kmalloc(sizeof(Task));

    if (strlen(name) > TASK_NAME_MAX) {
        ks.warn("Task name is longer than the maximum allowed (%i). The name will be truncated.", TASK_NAME_MAX);
        memory_copy(name, task->name, TASK_NAME_MAX);
    } else {
        strcpy(name, task->name);
    }

    task->status = TASK_EMBRYO;
    task->user = user;
    task->lock = NewLock;
    task->space = NewSpace();
    task->context = NewContext();

    task_set_stack(task);
    return task;
}

void DestroyTask(Task* task) {
    DestroySpace(task->space);
    DestroyContext(task->context);
    kfree(task);
}
