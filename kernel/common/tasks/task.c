#include "task.h"
#include "context.h"
#include "../kservice.h"
#include "../memory/memory.h"
#include "../memory/space.h"
#include <liballoc.h>
#include <string.h>

// === PRIVATE FUNCTIONS ========================

void task_set_name(Task* task, char* name) {
    if (strlen(name) > TASK_NAME_MAX) {
        ks.warn("Task name is longer than the maximum allowed (%i). The name will be truncated.", TASK_NAME_MAX);
        memory_copy((uint8_t*)name, (uint8_t*)task->name, TASK_NAME_MAX);
        task->name[TASK_NAME_MAX] = '\0';
    } else {
        strcpy(name, task->name);
    }
}

// === PUBLIC FUNCTIONS =========================

Task* NewTask(char* name, bool user) {
    Task* task = (Task*)kmalloc(sizeof(Task));

    task_set_name(task, name);

    task->status = TASK_EMBRYO;
    task->user = user;
    task->lock = NewLock;
    task->space = NewSpace();
    task->context = NewContext();

    task_set_stack(task);
    return task;
}

void DestroyTask(Task* task) {
    kfree(task->stack_base);
    DestroySpace(task->space);
    DestroyContext(task->context);
    kfree(task);
}
