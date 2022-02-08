#include "space.h"
#include "mem_virt.h"
#include "kernel/common/memory/space.h"
#include <liballoc.h>
#include <_null.h>

// === PRIVATE FUNCTIONS ========================

// === PUBLIC FUNCTIONS =========================

Space* NewSpace() {
    Space* space = (Space*)kmalloc(sizeof(Space));
    space->page_table = NewPageTable();
    return space;
}

void DestroySpace(Space* space) {
    if (space->memory_ranges != nullptr) {

        MemoryRangeNode* node = space->memory_ranges;
        while (node->next != nullptr) { // loop and free the memory ranges
            vmm_free_memory(space->page_table, node->range.base, node->range.size);
            kfree(node->range.base);
            node = node->next;
        }
        
    }

    DestroyPageTable(space->page_table);
    kfree(space);
}

uintptr_t space_map(Space* space, uintptr_t phys_addr, size_t size, uintptr_t virt_addr) {
    // vmm_map_page(RECURSE_ACTIVE, phys_addr, size, );
}

void space_unmap(Space* space, uintptr_t virt_addr) {

}
