#include "scheduler.h"
#include "arch.h"
#include "task.h"
#include "kernel/common/cpu.h"
#include "kernel/common/kservice.h"
#include <stdbool.h>
#include <_null.h>
#include <linkedlist.h>
#include <neutrino/lock.h>
#include <neutrino/macros.h>

// === PRIVATE FUNCTIONS ========================

// --- CPU functions ----------------------------

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
    lock((Lock*)&(cpu->tasks.current->lock));
    lock((Lock*)&(cpu->tasks.next->lock));

    if (cpu->tasks.current->status == TASK_RUNNING)
        cpu->tasks.current->status = TASK_READY;
    
    // swap
    Task* temp = cpu->tasks.current;
    cpu->tasks.current = cpu->tasks.next;
    cpu->tasks.next = temp;

    unlock((Lock*)&(cpu->tasks.next->lock));
    unlock((Lock*)&(cpu->tasks.current->lock));
    return;
}

void cpu_next(volatile Cpu* cpu) {
    lock((Lock*)&(cpu->tasks.next->lock));
    cpu->tasks.current = cpu->tasks.next;
    cpu->tasks.next = nullptr;
    unlock((Lock*)&(cpu->tasks.current->lock));
    return;
}

// --- Queue management -------------------------

// *Get if the GTQ is empty
// @return true if the GTQ is empty, false otherwise
bool queue_is_empty() {
    LockOperation(scheduler.gtq_lock, bool empty = list_is_empty(scheduler.gtq));
    return empty;
}

Task* unoptimized queue_assign(Task* task_slot, bool ignore_affinity, uint16_t cpu_id) {
    lock(&(scheduler.gtq_lock));

    List* p = scheduler.gtq;
    Task* t = nullptr;
    int t_index = 0;

    while (p != nullptr) {
        t = list_get_value(p);
        // check if task is unlocked, and acquire lock if so
        if (t->lock.flag == UNLOCKED) {
            lock(&(t->lock));

            // if task is runnable check for assignable
            if (IsTaskRunnable(t)) {
                // check for affinity
                if (!ignore_affinity) {
                    if (t->cpu_affinity.cpu_id != cpu_id) {
                        if (t->cpu_affinity.affinity_ignored == false)
                            t->cpu_affinity.affinity_ignored = true;
                        else
                            break;      // quit cycle, p is the task to be assigned
                    }    
                }

                break;      // affinity is unnecessary, p is the task to be assigned
            }

            unlock(&(t->lock));
        }
        
        t = nullptr;
        p = p->next;
        t_index++;
    }

    // ! check for empty task
    if (t == nullptr) return nullptr;

    // assign task and reset affinity
    task_slot = t;
    task_slot->cpu_affinity.cpu_id = cpu_id;
    task_slot->cpu_affinity.affinity_ignored = false;

    // remove from gtq
    scheduler.gtq = list_delete_at(scheduler.gtq, t_index);
    unlock(&(scheduler.gtq_lock));

    // ! unacquire the task lock
    unlock(&task_slot->lock);

    return task_slot;
}

void queue_put(Task* task) {
    lock(&(scheduler.gtq_lock));
    lock(&(task->lock));

    if (task->status == TASK_RUNNING)
        task->status = TASK_READY;

    scheduler.gtq = list_append(scheduler.gtq, task);

    unlock(&(task->lock));
    unlock(&(scheduler.gtq_lock));
}

// --- Scheduler default tasks ------------------

void cpu_idle() {
    arch_idle();
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
    scheduler.gtq_lock = NewLock;

    ks.log("Scheduler initialized");
    scheduler.ready = true;
}

void sched_start(Task* task, uintptr_t entry_point) {
    lock(&task->lock);
    context_init(task->context, entry_point, PROCESS_STACK_BASE + PROCESS_STACK_SIZE - sizeof(uintptr_t), PROCESS_STACK_BASE, (ContextFlags){.user = task->user});
    task->status = TASK_NEW;
    unlock(&task->lock);

    queue_put(task);
}

void unoptimized sched_cycle(volatile Cpu* cpu) {
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
            cpu->tasks.current = queue_assign(cpu->tasks.current, true, cpu->id);
        }
    } else {
        if (queue_is_empty()) {
            if (!cpu_is_next_free(cpu)) {
                // if the cpu is busy, the GTQ is empty and the next slot is filled, then swap current and next.
                if (cpu->tasks.current == nullptr)
                    cpu_next(cpu);
                else
                    cpu_swap_task(cpu);
            }
            
        } else {
            // if the cpu is busy and the GTQ has something pending move current task to GTQ
            if (cpu->tasks.current != nullptr)
                queue_put(cpu->tasks.current);

            if (cpu_is_next_free(cpu)) 
                // if the next slot is free assign the current
                cpu->tasks.current = queue_assign(cpu->tasks.current, true, cpu->id);
            else 
                // if the next slot is filled put the next in the current slot
                cpu_next(cpu);
            
            // try to assign the next
            cpu->tasks.next = queue_assign(cpu->tasks.next, false, cpu->id);
        }
    }

    // if current task is empty, it becomes an idle task
    if (cpu->tasks.current == nullptr) 
        cpu->tasks.current = cpu->tasks.idle;

    cpu->tasks.current->status = TASK_RUNNING;
    return;
}

void unoptimized sched_terminate() {
    disable_interrupts();
    Cpu* cpu = get_current_cpu();
    LockOperation(cpu->tasks.current->lock, cpu->tasks.current->status = TASK_ZOMBIE);
    ks.log("Task ID %d (%c) on CPU %d terminated.", 
            cpu->tasks.current->pid, cpu->tasks.current->name, cpu->id);
    enable_interrupts();
    arch_idle();
}
