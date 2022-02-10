#pragma once
#include "paging.h"
#include <stdint.h>
#include <size_t.h>
#include <stdbool.h>
#include <libs/limine/stivale2.h>

#define MAP_EARLY_FUNC(page, paddr, vaddr, wr, us) (early) ? vmm_map_page_early(page, paddr, vaddr, wr, us) : vmm_map_page(page, paddr, vaddr, wr, us);

struct memory_virtual {
    uint8_t address_size; 
    PageTable* kernel_page_physaddr;
    PageTable* stivale2_page_physaddr;
};

struct memory_virtual vmm;

void init_vmm();
void init_vmm_on_ap(struct stivale2_smp_info* info);

bool vmm_unmap_page(PageTable* table, uint64_t virt_addr);
void vmm_map_page(PageTable* table, uint64_t phys_addr, uint64_t virt_addr, bool writable, bool user);

uintptr_t vmm_allocate_memory(PageTableEntry* table, size_t blocks, bool writable, bool user);
bool vmm_free_memory(PageTableEntry* table, uint64_t addr, size_t blocks);
void vmm_map_mmio(uint64_t mmio_addr, size_t blocks);

PageTable* NewPageTable();
void DestroyPageTable(PageTable* page_table);
void vmm_switch_space(PageTable* page_table);
