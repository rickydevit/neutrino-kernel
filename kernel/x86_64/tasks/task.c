#include "task.h"
#include "../arch.h"
#include "../memory/mem_phys.h"
#include "../memory/mem_virt.h"
#include "kernel/common/tasks/scheduler.h"
#include <neutrino/macros.h>
#include <stdbool.h>

void unoptimized task_set_stack(Task* task, bool user) {
    task->stack_base = (uintptr_t)pmm_alloc_series(PROCESS_STACK_SIZE / PAGE_SIZE);     

    // set task head to terminator 
    uintptr_t task_terminator = task->stack_base + PROCESS_STACK_SIZE - sizeof(uintptr_t);
    vmm_map_page(0, task_terminator, get_mem_address(task_terminator), PageKernelWrite);

    *(uintptr_t*)get_mem_address(task_terminator) = (uintptr_t)sched_terminate;
    
    vmm_unmap_page(0, get_mem_address(task_terminator));

    // map the stack into task space
    space_map(task->space, task->stack_base, PROCESS_STACK_BASE, PROCESS_STACK_SIZE / PAGE_SIZE, 
              (user) ? MAP_USER | MAP_WRITABLE : MAP_WRITABLE);
}
