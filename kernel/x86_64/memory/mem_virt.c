#include "mem_virt.h"
#include "mem_phys.h"
#include "paging.h"
#include "../cpuid.h"
#include "../arch.h"
#include "kernel/common/kservice.h"
#include "libs/libc/stdbool.h"

// === PRIVATE FUNCTIONS ========================

// *Get a page entry in the given page table
// @param table the page table to search the entry in
// @param entry the entry to be found
// @return a pointer to the page entry requested, or 0 if not found
page_table* vmm_get_entry(page_table* table, uint64_t entry) {
    if (IS_PRESENT(table->entries[entry])) 
        return get_mem_address(GET_PHYSICAL_ADDRESS(table->entries[entry]));
    return 0;
}

// *Create a new page table entry in the given table at the given entry with the specified writable and user flags
// @param table the table to add the entry to
// @param entry the index of the entry to be added
// @param writable flag to indicate whether the newly created entry should be writable or not
// @param user flags to indicate whether the newly created entry should be accessible from userspace or not
// @return the address of the newly created entry
page_table* vmm_create_entry(page_table* table, uint64_t entry, bool writable, bool user) {
    page_table* pt = (page_table*)get_mem_address(pmm_alloc());
    table->entries[entry] = page_create(get_rmem_address(pt), writable, user);
    return pt;
}

// *Get or create the entry in the given page table. If the entry already exists in the page table, its address
// *is returned. Otherwise a new entry is created and returned.
// @param table the table to add the entry to
// @param entry the index of the entry to be added
// @param writable flag to indicate whether the newly created entry should be writable or not
// @param user flags to indicate whether the newly created entry should be accessible from userspace or not
// @return the address of the existing table or the newly created table
page_table* vmm_get_or_create_entry(page_table* table, uint64_t entry, bool writable, bool user) {
    if (IS_PRESENT(table->entries[entry]))
        return get_mem_address(GET_PHYSICAL_ADDRESS(table->entries[entry]));
    else 
        return vmm_create_entry(table, entry, writable, user);
}

// *Return the physical address given a virtual address
// @param virt the virtual address
// @return the physical address associated to the virtual address
uint64_t vmm_get_phys_address(uint64_t virt) {
    page_table* pdpt = vmm_get_entry(vmm.active_page, (uint64_t)GET_PL4_INDEX(virt));
    if (pdpt == 0) ks.warn("Physical address for virtual address %x is invalid", virt);
    
    page_table* pdir = vmm_get_entry(pdpt, (uint64_t)GET_DPT_INDEX(virt));
    if (pdir == 0) ks.warn("Physical address for virtual address %x is invalid", virt);
   
    page_table* ptab = vmm_get_entry(pdir, (uint64_t)GET_DIR_INDEX(virt));
    if (ptab == 0) ks.warn("physical address for virtual address %x is invalid", virt);

    return GET_PHYSICAL_ADDRESS(ptab->entries[(uint64_t)GET_TAB_INDEX(virt)]) + GET_PAGE_OFFSET(virt);
}

// *Return a new page table of 512 entries all set to not-present 
// @return the new page table pointer
page_table *vmm_new_table() {
    page_table *pl4 = (page_table*) pmm_alloc();
    for (int i = 0; i < PAGE_PL4_ENTRIES; i++) pl4->entries[i] = 0;

    return pl4;
}

// *Map a physical address to a virtual page address
// @param table the table to map the address into
// @param phys_addr the physical address to map the virtual address to
// @param virt_addr the virtual address to map the physical address to
// @param writable flag to indicate whether the newly created entry should be writable or not
// @param user flags to indicate whether the newly created entry should be accessible from userspace or not
void vmm_map_page(page_table* table, uint64_t phys_addr, uint64_t virt_addr, bool writable, bool user) {
    uint64_t pl4_entry = GET_PL4_INDEX(virt_addr);
    uint64_t dpt_entry = GET_DPT_INDEX(virt_addr);
    uint64_t pd_entry = GET_DIR_INDEX(virt_addr);
    uint64_t pt_entry = GET_TAB_INDEX(virt_addr);

    if (pl4_entry != 0) ks.dbg("pml4: %i pdpt: %i pd: %i pt: %i", pl4_entry, dpt_entry, pd_entry, pt_entry);

    page_table *pdpt, *pd, *pt;
    pdpt = vmm_get_or_create_entry(table, pl4_entry, true, true);
    // if (pl4_entry != 0) ks.dbg("pdpt at %x", pdpt);
    pd = vmm_get_or_create_entry(pdpt, dpt_entry, true, true);
    // if (pl4_entry != 0) ks.dbg("dp at %x", pd);
    pt = vmm_get_or_create_entry(pd, pd_entry, true, true);
    // if (pl4_entry != 0) ks.dbg("pt at %x", pt);

    pt->entries[pt_entry] = page_create(phys_addr, writable, user);
}

// === PUBLIC FUNCTIONS =========================

void init_vmm() {
    ks.log("Initializing VMM...");

    vmm.address_size = get_physical_address_length();
    vmm.active_page = (page_table*) read_cr3();

    // prepare a pml4 table for the kernel address space
    page_table* kernel_pml4 = vmm_new_table();

    // identity map the first 2M of physical memory
    for (int i = 0; i < PHYSMEM_2MEGS / PAGE_SIZE; i++)
        vmm_map_page(kernel_pml4, i*PAGE_SIZE, i*PAGE_SIZE, true, false);

    // map the page itself in the 511st pml4 entry
    ks.dbg("mapping page map inside itself: %x to %x", kernel_pml4, RECURSIVE_PAGE_ADDRESS);
    vmm_map_page(kernel_pml4, kernel_pml4, RECURSIVE_PAGE_ADDRESS, true, false);

    // map the kernel in the 511th pml4 entry
    for (int ind = 0; ind < pmm.regions_count; ind++) {
        if (pmm.regions[ind].type == MEMORY_REGION_USABLE ||
            pmm.regions[ind].type == MEMORY_REGION_INVALID ||
            pmm.regions[ind].type == MEMORY_REGION_FRAMEBUFFER) continue;

        uint64_t aligned_base = pmm.regions[ind].base - pmm.regions[ind].base % PAGE_SIZE;
        uint64_t aligned_size = ((pmm.regions[ind].size / PAGE_SIZE) + 1) * PAGE_SIZE;

        ks.dbg("region is %i bytes long, needing %i pages. Aligned base is %x, aligned length is %x", 
            pmm.regions[ind].size, pmm.regions[ind].size / PAGE_SIZE + 1, aligned_base, aligned_size);

        for (int i = 0; i * PAGE_SIZE < aligned_size; i++) {
            uint64_t addr = aligned_base + i * PAGE_SIZE;
            // ks.dbg("mapping region from %x to %x", addr, get_kern_address(addr));
            vmm_map_page(kernel_pml4, addr, get_kern_address(addr), true, false);
        } 
    }

    // map other things that may be useful (like ACPI tables etc)

    // give CR3 the kernel pml4 address
    write_cr3((uint64_t)kernel_pml4);
    ks.log("HELLO PAGING WORLD!");
}
