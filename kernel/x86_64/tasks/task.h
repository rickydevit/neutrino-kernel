#pragma once

#include <neutrino/macros.h>
#include <stdint.h>
#include "memory/mem_virt.h"
#include "device/apic.h"
#include "kernel/common/tasks/task.h"

static inline volatile_fun Task* NewIdleTask(uintptr_t entry_point) {
    Task* idle = NewTask("idle", false);
    space_map(idle->space, apic.apic_addr, 3, apic.apic_addr);  // map the apic 
    context_init(idle->context, entry_point, PROCESS_STACK_BASE + PROCESS_STACK_SIZE, PROCESS_STACK_BASE, (ContextFlags)0);

    idle->status = TASK_READY;

    return idle;
}
