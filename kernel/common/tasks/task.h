#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <neutrino/lock.h>
#include <neutrino/macros.h>
#include <_null.h>
#include "context.h"
#include "../memory/space.h"

typedef enum __task_status {
    TASK_EMBRYO,            // task is creating its Context and Space
    TASK_NEW,               // task has never been executed before
    TASK_READY,             // task is ready for execution
    TASK_RUNNING,           // task is running
    TASK_ZOMBIE             // task is ended and freeing its resources
} TaskStatus;

#define TASK_NAME_MAX 64
#define PROCESS_STACK_SIZE  0x4000
#define PROCESS_STACK_BASE  0x80000000000

typedef struct __task {
    uint32_t pid;
    char name[TASK_NAME_MAX+1];

    TaskStatus status;
    struct {
        uint16_t cpu_id;
        bool affinity_ignored;
    } packed cpu_affinity;

    // flags
    Lock lock;
    bool user;
    bool in_syscall;
    bool in_io;

    Context* context;       // ! must SAVE before every scheduler cycle and RESTORE thereafter
    Space* space;           // ! must SWITCH after every scheduler cycle

    uintptr_t stack_base;
} Task;

#define IsTaskRunnable(task)    ((task)->status == TASK_READY || (task)->status == TASK_NEW)
#define IsTaskNeverRun(task)    ((task)->status == TASK_EMBRYO || (task)->status == TASK_NEW)

Task* NewTask(char* name, bool user);
Task* NewIdleTask(uintptr_t entry_point);
void DestroyTask(Task* task);

#ifdef __x86_64
#include "kernel/x86_64/tasks/task.h"
#else
#error "Unsupported platform"
#endif

void task_set_stack(Task* task);
