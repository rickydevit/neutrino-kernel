#include <neutrino/lock.h>
#include <stdbool.h>
#include <neutrino/syscall.h>

// #ifdef __x86_64
// #include "kernel/x86_64/memory/mem_virt.h"
// #include "kernel/x86_64/memory/mem_phys.h"
// #else 
// #error "Unsupported platform"
// #endif

#define LIBALLOC_HEAP_START 0xffffffff80000000
#define LIBALLOC_HEAP_END   0xffffffffffffffff

static Lock kalloc_lock = NewLock;

int liballoc_lock() {
    lock(&kalloc_lock);
    return 0;
}

int liballoc_unlock() {
    unlock(&kalloc_lock);
    return 0;
}

void* liballoc_alloc(size_t pages) {
    SCAllocArgs alloc_args = (SCAllocArgs){.size = pages};
#ifdef __kernel
    // uintptr_t addr = vmm_allocate_heap(pages, false);
    alloc_args.user = false;
    sys_alloc(&alloc_args);
#else
    alloc_args.user = true;
    neutrino_alloc(&alloc_args);
#endif    

    return (void*)alloc_args.pointer;
}

int liballoc_free(void* address, size_t pages) {
    // if (vmm_free_memory(0, (uintptr_t)address, pages) == true) return 0;
    return 1;
}

