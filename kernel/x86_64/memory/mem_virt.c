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
        return GET_PHYSICAL_ADDRESS(table->entries[entry]);
    return 0;
}

// *Create a new page table entry in the given table at the given entry with the specified writable and user flags
// @param table the table to add the entry to
// @param entry the index of the entry to be added
// @param writable flag to indicate whether the newly created entry should be writable or not
// @param user flags to indicate whether the newly created entry should be accessible from userspace or not
// @return the address of the newly created entry
page_table* vmm_create_entry(page_table* table, uint64_t entry, bool writable, bool user) {
    page_table* pt = (page_table*)pmm_alloc();
    table->entries[entry] = page_create(pt, writable, user);
    return pt;
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

// === PUBLIC FUNCTIONS =========================

void init_vmm() {
    ks.log("Initializing VMM...");

    vmm.address_size = get_physical_address_length();
    vmm.active_page = (page_table*) read_cr3();

    // prepare a pml4 table for the kernel address space
    page_table* pml4 = vmm_new_table();
    page_table* test = vmm_create_entry(pml4, 255, true, false);

    ks.dbg("entry: %b addr: %x", pml4->entries[255], GET_PHYSICAL_ADDRESS(pml4->entries[255]));
    
    // map the kernel in the 256th pml4 entry
    ks.dbg("kernel is %i, needing %i pages", pmm_get_region_by_type(MEMORY_REGION_KERNEL).size, pmm_get_region_by_type(MEMORY_REGION_KERNEL).size / PAGE_SIZE + 1);


    // map other things that may be useful (like ACPI tables etc)

    // give CR3 the kernel pml4 address

}
