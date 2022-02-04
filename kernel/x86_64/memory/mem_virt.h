#pragma once
#include "paging.h"
#include <stdint.h>
#include <size_t.h>
#include <stdbool.h>
#include <libs/limine/stivale2.h>

#define USE_ACTIVE_PAGE 0x0

typedef struct __paging_path {
    uint64_t pl4;
    uint64_t dpt;
    uint64_t pd;
    uint64_t pt;
} PagingPath;

#define GetPagingPath(virt_addr) (PagingPath){GET_PL4_INDEX(virt_addr), GET_DPT_INDEX(virt_addr), GET_DIR_INDEX(virt_addr), GET_TAB_INDEX(virt_addr)}

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
void vmm_map_page_in_active_table(uint64_t phys_addr, uint64_t virt_addr, bool writable, bool user);
bool vmm_unmap_page_in_active_table(uint64_t virt_addr);

uintptr_t vmm_allocate_memory(PageTableEntry* table, size_t blocks, bool writable, bool user);
bool vmm_free_memory(PageTableEntry* table, uint64_t addr, size_t blocks);
void vmm_map_mmio(uint64_t mmio_addr, size_t blocks);
