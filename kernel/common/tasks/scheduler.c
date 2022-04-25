#include "scheduler.h"
#include "task.h"
#include "kernel/common/cpu.h"
#include "kernel/common/kservice.h"
#include <stdbool.h>
#include <_null.h>
#include <neutrino/lock.h>
#include <neutrino/macros.h>

// === PRIVATE FUNCTIONS ========================

// *Get if the given CPU is currently idle
// @param cpu the CPU to check 
// @return true if both current and next tasks are cleared, false otherwise
bool cpu_is_idle(volatile Cpu* cpu) {
    return (cpu->tasks.current == nullptr || cpu->tasks.current == cpu->tasks.idle) 
            && (cpu->tasks.next == nullptr);
}

// *Get it the given CPU is Next-free
// @param cpu the CPU to check 
// @return true if next task is cleared, false otherwise
bool cpu_is_next_free(volatile Cpu* cpu) {
    return (cpu->tasks.next == nullptr);
}

bool cpu_peek_other(volatile Cpu* cpu) {
    ks.fatal(FatalError(NOT_IMPLEMENTED, "cpu_peek_other() not implemented."));
    return false;
}

void cpu_swap_task(volatile Cpu* cpu) {
    lock((Lock*)&(cpu->tasks.current));
    lock((Lock*)&(cpu->tasks.next));

    if (cpu->tasks.current->status == TASK_RUNNING)
        cpu->tasks.current->status = TASK_READY;
    
    // swap
    Task* temp = cpu->tasks.current;
    cpu->tasks.current = cpu->tasks.next;
    cpu->tasks.next = temp;

    unlock((Lock*)&(cpu->tasks.next));
    unlock((Lock*)&(cpu->tasks.current));
    return;
}

void cpu_next(volatile Cpu* cpu) {
    lock((Lock*)&(cpu->tasks.next));
    cpu->tasks.current = cpu->tasks.next;
    unlock((Lock*)&(cpu->tasks.current));
    return;
}

// *Get if the GTQ is empty
// @return true if the GTQ is empty, false otherwise
bool queue_is_empty() {
    LockOperation(scheduler.gtq_lock, bool empty = scheduler.gtq == nullptr);
    return empty;
}

bool queue_assign(Task* task_slot, bool ignore_affinity, uint16_t cpu_id) {
    lock(&(scheduler.gtq_lock));

    GlobalTaskQueueNode* p = scheduler.gtq;
    while (p != nullptr) {
        // check if task is unlocked, and acquire lock if so
        if (p->task->lock.flag == UNLOCKED) {
            lock(&(p->task->lock));

            // if task is runnable check for assignable
            if (IsTaskRunnable(p->task)) {
                // check for affinity
                if (!ignore_affinity) {
                    if (p->task->cpu_affinity.cpu_id != cpu_id) {
                        if (p->task->cpu_affinity.affinity_ignored == false)
                            p->task->cpu_affinity.affinity_ignored = true;
                        else
                            break;      // quit cycle, p is the task to be assigned
                    }    
                }

                break;      // affinity is unnecessary, p is the task to be assigned
            }

            unlock(&(p->task->lock));
        }
        
        p = p->next;
    }
    unlock(&(scheduler.gtq_lock));

    // ! check for empty task
    if (p->task == nullptr) return false;

    // assign task and reset affinity
    task_slot = p->task;
    task_slot->cpu_affinity.cpu_id = cpu_id;
    task_slot->cpu_affinity.affinity_ignored = false;

    // ! unacquire the task lock
    unlock(&task_slot->lock);

    return true;
}

void queue_put(Task* task) {
    lock(&(scheduler.gtq_lock));
    lock(&(task->lock));

    if (task->status == TASK_RUNNING)
        task->status = TASK_READY;

    scheduler.gtq_last->next = scheduler.gtq_last;
    scheduler.gtq_last->task = task;

    unlock(&(task->lock));
    unlock(&(scheduler.gtq_lock));
    return;
}

// --- Scheduler default tasks ------------------

void cpu_idle() {
    asm volatile ("mov %0, %%eax" : : "g" (0xff0000 | get_current_cpu()->id));
    asm volatile ("int %0" : : "g" (3));
    while (true) asm volatile ("hlt");
}

// === PUBLIC FUNCTIONS =========================

void unoptimized init_scheduler() {
    ks.log("Initializing scheduler...");
    scheduler.ready = false;

    for (size_t i = 0; i < get_cpu_count(); i++) {
        volatile Cpu* cpu = get_cpu(i);
        cpu->tasks.idle = NewIdleTask((uintptr_t)cpu_idle);
        cpu->tasks.is_switching = NewLock;
    }

    scheduler.gtq = nullptr;
    scheduler.gtq_last = nullptr;
    scheduler.gtq_lock = NewLock;

    ks.log("Scheduler initialized");
    scheduler.ready = true;

    enable_interrupts();
    for (;;) asm volatile("hlt");
}

void sched_start(Task* task) {
    // todo: implement correct start
    for (size_t i = 0; i < get_cpu_count(); i++)
        get_cpu(i)->tasks.current = get_cpu(i)->tasks.idle;
}

void sched_cycle(volatile Cpu* cpu) {
    if (cpu->tasks.current != nullptr) {
        if (cpu->tasks.current->status == TASK_ZOMBIE) {
            DestroyTask(cpu->tasks.current);
            cpu->tasks.current = nullptr;
        }
    }

    if (cpu_is_idle(cpu)) {
        if (queue_is_empty()) {                         
            // if cpu is idle and queue is empty, try peeking from other cpus. 
            // cpu_peek_other(cpu); // todo when implementing multi-core
        } else {                                        
            // if the cpu is idle but the GTQ has something pending, assign to cpu from the queue.
            queue_assign(cpu->tasks.current, true, cpu->id);
        }
    } else {
        if (queue_is_empty()) {
            if (!cpu_is_next_free(cpu))
                // if the cpu is busy, the GTQ is empty and the next slot is filled, then swap current and next.
                cpu_swap_task(cpu);
            
        } else {
            // if the cpu is busy and the GTQ has something pending move current task to GTQ
            queue_put(cpu->tasks.current);

            if (cpu_is_next_free(cpu)) 
                // if the next slot is free assign the current
                queue_assign(cpu->tasks.current, true, cpu->id);
            else 
                // if the next slot is filled put the next in the current slot
                cpu_next(cpu);
            
            // try to assign the next
            queue_assign(cpu->tasks.next, false, cpu->id);
        }
    }

    // if current task is empty, it becomes an idle task
    if (cpu->tasks.current == nullptr) 
        cpu->tasks.current = cpu->tasks.idle;

    cpu->tasks.current->status = TASK_RUNNING;
    return;
}
