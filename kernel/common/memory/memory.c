#include "memory.h"
#include <stdint.h>
#include <size_t.h>
#include <neutrino/macros.h>

// *Copy [nbytes] bytes from the [source] memory pointer to the [dest] memory pointer
// @param source the source memory pointer
// @param dest the destination memory pointer
// @param nbytes the number of bytes to copy
void unoptimized memory_copy(uint8_t *source, uint8_t *dest, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        *(dest + i) = *(source + i);
    }
}

// *Set [len] bytes at the [dest] memory location to the [val] value
// @param dest the memory location to set bytes
// @param val the value to set the bytes to
// @param len the number of bytes to set
void unoptimized memory_set(uint8_t *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

#ifdef __x86_64
#include "kernel/x86_64/memory/mem_virt.h"
#include "kernel/x86_64/arch.h"

VirtualMapping memory_allocate(size_t size) {
    uintptr_t vaddr = vmm_allocate_memory(0, (size / PAGE_SIZE)+1, PageKernelWrite);
    
    return (VirtualMapping) {
        .physical = {
            .base = get_rmem_address(vaddr),
            .size = size
        },
        .virtual_base = vaddr
    };
}

void memory_free(VirtualMapping mapping) {
    vmm_free_memory(0, mapping.virtual_base, (mapping.physical.size / PAGE_SIZE)+1);
}

#else 
#error "Unsupported platform"
#endif
