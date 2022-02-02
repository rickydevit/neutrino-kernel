#include <neutrino/lock.h>

#ifdef __x86_64
#include "kernel/x86_64/memory/mem_virt.h"
#include "kernel/x86_64/memory/mem_phys.h"
#endif

#define LIBALLOC_HEAP_START 0xffffffff80000000
#define LIBALLOC_HEAP_END   0xffffffffffffffff

static lock_t kalloc_lock = NewLock;

int liballoc_lock() {
    lock(&kalloc_lock);
    return 0;
}

int liballoc_unlock() {
    unlock(&kalloc_lock);
    return 0;
}

void* liballoc_alloc(size_t pages) {
    vmm_allocate_memory(USE_ACTIVE_PAGE, pages, true, false);
}

int liballoc_free(void* address, size_t pages) {
    if (vmm_free_memory(USE_ACTIVE_PAGE, address, pages) == true) return 0;
    return 1;
}

