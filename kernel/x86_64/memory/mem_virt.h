#pragma once
#include "paging.h"
#include "libs/libc/stdint.h"
#include "libs/libc/stdbool.h"

struct memory_virtual {
    uint8_t address_size; 
    page_table* active_page;
};

struct memory_virtual vmm;

bool vmm_unmap_page(page_table* table, uint64_t virt_addr);
void vmm_map_page(page_table* table, uint64_t phys_addr, uint64_t virt_addr, bool writable, bool user);
void vmm_map_page_in_active_table(uint64_t phys_addr, uint64_t virt_addr, bool writable, bool user);
bool vmm_unmap_page_in_active_table(uint64_t virt_addr);
