#include "task.h"
#include "../memory/mem_phys.h"

void task_set_stack(Task* task) {
    task->stack_base = (uintptr_t)pmm_alloc_series(PROCESS_STACK_SIZE / PAGE_SIZE);       
    space_map(task->space, task->stack_base, PROCESS_STACK_SIZE / PAGE_SIZE, PROCESS_STACK_BASE);
}
