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
bool cpu_is_idle(Cpu* cpu) {
    return (cpu->tasks.current == nullptr) && (cpu->tasks.next == nullptr);
}

// *Get it the given CPU is Next-free
// @param cpu the CPU to check 
// @return true if next task is cleared, false otherwise
bool cpu_is_next_free(Cpu* cpu) {
    return (cpu->tasks.next == nullptr);
}

bool cpu_peek_other(Cpu* cpu) {
    ks.fatal(FatalError(NOT_IMPLEMENTED, "cpu_peek_other() not implemented."));
    return false;
}

void cpu_swap_task(Cpu* cpu) {
    ks.fatal(FatalError(NOT_IMPLEMENTED, "cpu_swap_task() not implemented."));
    return;
}

void cpu_next(Cpu* cpu) {
    ks.fatal(FatalError(NOT_IMPLEMENTED, "cpu_next() not implemented."));
    return;
}

// *Get if the GTQ is empty
// @return true if the GTQ is empty, false otherwise
bool queue_is_empty() {
    LockOperation(scheduler.gtq_lock, bool empty = scheduler.gtq == nullptr);
    return empty;
}

bool queue_assign(Task* task_slot, bool ignore_affinity) {
    ks.fatal(FatalError(NOT_IMPLEMENTED, "queue_assign() not implemented."));
    return false;
}

void queue_put(Task* task) {
    ks.fatal(FatalError(NOT_IMPLEMENTED, "queue_put() not implemented."));
    return;
}

// --- Scheduler default tasks ------------------

void cpu_idle() {
    ks.dbg("TASK IDLE");
    while (true)
        asm volatile ("hlt");
}

// === PUBLIC FUNCTIONS =========================

void volatile_fun init_scheduler() {
    ks.log("Initializing scheduler...");
    scheduler.ready = false;

    // set idle task
    scheduler.idle = NewIdleTask((uintptr_t)cpu_idle);

    scheduler.gtq = nullptr;
    scheduler.gtq_last = nullptr;
    scheduler.gtq_lock = NewLock;

    ks.log("Scheduler initialized");
    scheduler.ready = true;
}

void sched_cycle(Cpu* cpu) {
    // ks.log("sched_cycle() : cpu: %u", cpu->id);
    cpu->tasks.current = scheduler.idle;
    return;

    if (cpu_is_idle(cpu)) {
        if (queue_is_empty()) {                         
            // if cpu is idle and queue is empty, try peeking from other cpus. 
            cpu_peek_other(cpu);
        } else {                                        
            // if the cpu is idle but the GTQ has something pending, assign to cpu from the queue.
            queue_assign(cpu->tasks.current, true);
        }
    } else {
        if (queue_is_empty()) {
            if (cpu_is_next_free(cpu))
                // if the cpu is busy, the GTQ is empty and the next slot is filled, then swap current and next.
                cpu_swap_task(cpu);
            
        } else {
            // if the cpu is busy and the GTQ has something pending move current task to GTQ
            queue_put(cpu->tasks.current);

            if (cpu_is_next_free(cpu)) 
                // if the next slot is free assign the current
                queue_assign(cpu->tasks.current, true);
            else 
                // if the next slot is filled put the next in the current slot
                cpu_next(cpu);
            
            // try to assign the next
            queue_assign(cpu->tasks.next, false);
        }
    }

    // if current task is empty, it becomes an idle task
    if (cpu->tasks.current == nullptr) 
        cpu->tasks.current = scheduler.idle;
}
