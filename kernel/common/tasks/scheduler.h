#pragma once

#include "task.h"
#include "../cpu.h"
#include <_null.h>
#include <linkedlist.h>
#include <neutrino/lock.h>
#include <neutrino/macros.h>

typedef union __global_task_queue_node {
    Task* task;
    union __global_task_queue_node* next;
} packed GlobalTaskQueueNode;

typedef struct __scheduler {
    bool ready;
    List* gtq;
    Lock gtq_lock;                  // locked when a CPU is accessing the GTQ
} Scheduler;

Scheduler scheduler;

void sched_cycle(volatile Cpu* cpu);
void sched_start(Task* task, uintptr_t entry_point);
void init_scheduler();
