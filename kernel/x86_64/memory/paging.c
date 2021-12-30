#include "paging.h"
#include "stdbool.h"
 
// *Check if paging is enabled
// @return true if paging is enabled, false otherwise
bool is_paging_enabled() {
    uint32_t result;
    __asm__("mov %%cr0, %%rbx" : "=b" (result) : );
    return (result >> 31) == 1;
}

// *Disable paging by setting CR0's PG to 0
void disable_paging() {
    uint32_t result;
    __asm__("mov %%cr0, %%rbx" : "=b" (result) : );
    __asm__("mov %%rbx, %%cr0" :  : "b" (~(1 << 31) & result));
}

// *Read the current value of the CR3 register
// @return the current value of the CR3 register
uint64_t read_cr3() {
    uint64_t value;
    __asm__("mov %%cr3, %%rax" : "=a"(value)); 
    return value;
}

// *Write the given value to the CR3 register
// @param value the value to write to CR3 register
void write_cr3(uint64_t value) {
    __asm__("mov %0, %%cr3" : : "r"(value));
}

page_table_e page_create(uint64_t addr, bool writable, bool user) {
    page_table_e pt = addr & ADDRESS_MASK;

    if (writable) page_set_bit(&pt, WRITABLE_BIT_OFFSET);
    if (user) page_set_bit(&pt, USERSPACE_BIT_OFFSET);
    page_set_bit(&pt, PRESENT_BIT_OFFSET);

    return pt;
}
