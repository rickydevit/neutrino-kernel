#pragma once
#include "paging.h"
#include "libs/libc/stdint.h"
#include "libs/libc/stdbool.h"
#include "thirdparty/stivale2.h"

struct memory_virtual {
    uint8_t address_size; 
    page_table* kernel_page_physaddr;
    page_table* stivale2_page_physaddr;
};

struct memory_virtual vmm;

void init_vmm();
void init_vmm_on_ap(struct stivale2_smp_info* info);
bool vmm_unmap_page(page_table* table, uint64_t virt_addr);
void vmm_map_page(page_table* table, uint64_t phys_addr, uint64_t virt_addr, bool writable, bool user);
void vmm_map_page_in_active_table(uint64_t phys_addr, uint64_t virt_addr, bool writable, bool user);
bool vmm_unmap_page_in_active_table(uint64_t virt_addr);
