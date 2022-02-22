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
    TASK_READY,             // task is ready for execution
    TASK_RUNNING,           // task is running
    TASK_IO,                // task is blocked by I/O 
    TASK_SYSCALL,           // task is executing a syscall
    TASK_ZOMBIE             // task is ended and freeing its resources
} TaskStatus;

#define TASK_NAME_MAX 64
#define PROCESS_STACK_SIZE  0x4000
#define PROCESS_STACK_BASE  0xc0000000

typedef struct __task {
    uint32_t pid;
    char name[TASK_NAME_MAX];

    TaskStatus status;
    struct {
        uint16_t cpu_id;
        bool affinity_ignored;
    } packed cpu_affinity;

    Lock lock;
    bool user;
    Context* context;       // ! must SAVE before every scheduler cycle and RESTORE thereafter
    Space* space;           // ! must SWITCH after every scheduler cycle

    uintptr_t stack;
} Task;

#define IsTaskRunnable(task)    ((task)->status == TASK_READY || (task)->status == TASK_SYSCALL)

Task* NewTask(const char* name, bool user);
void DestroyTask(Task* task);
