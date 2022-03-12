#include "mem_virt.h"
#include "mem_phys.h"
#include "paging.h"
#include "../smp.h"
#include "../cpuid.h"
#include "../arch.h"
#include "kernel/common/kservice.h"
#include "kernel/common/memory/memory.h"
#include <stdbool.h>
#include <libs/limine/stivale2.h>
#include <size_t.h>
#include <liballoc.h>

// === PRIVATE FUNCTIONS ========================

// -- Uutilities --------------------------------

// *Get a page entry in the given page table
// @param table the page table to search the entry in
// @param entry the entry to be found
// @return a pointer to the page entry requested, or 0 if not found
PageTable* vmm_get_entry(PageTable* table, uint64_t entry) {
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
PageTable* volatile_fun vmm_create_entry(PageTable* table, uint64_t entry, bool writable, bool user) {
    PageTable* pt = (PageTable*)get_mem_address(pmm_alloc());
    memory_set(get_rmem_address(pt), 0, PHYSMEM_BLOCK_SIZE);
    table->entries[entry] = page_create(get_rmem_address(pt), writable, user);
    vmm_refresh_paging();

    return get_rmem_address(pt);
}

// *Get or create the entry in the given page table. If the entry already exists in the page table, its address
// *is returned. Otherwise a new entry is created and returned.
// @param table the table to add the entry to
// @param entry the index of the entry to be added
// @param writable flag to indicate whether the newly created entry should be writable or not
// @param user flags to indicate whether the newly created entry should be accessible from userspace or not
// @return the address of the existing table or the newly created table
PageTable* vmm_get_or_create_entry(PageTable* table, uint64_t entry, bool writable, bool user) {
    if (IS_PRESENT(table->entries[entry])) {
        return GET_PHYSICAL_ADDRESS(table->entries[entry]);
    } else {
        return vmm_create_entry(table, entry, writable, user);
    }
}

// *Return the physical address given a virtual address
// @param virt the virtual address
// @return the physical address associated to the virtual address
uint64_t vmm_virt_to_phys(uint64_t virt) {
    PageTable* pdpt = vmm_get_entry(read_cr3(), (uint64_t)GET_PL4_INDEX(virt));
    if (pdpt == 0) ks.warn("Physical address for virtual address %x is invalid", virt);
    
    PageTable* pdir = vmm_get_entry(pdpt, (uint64_t)GET_DPT_INDEX(virt));
    if (pdir == 0) ks.warn("Physical address for virtual address %x is invalid", virt);
   
    PageTable* ptab = vmm_get_entry(pdir, (uint64_t)GET_DIR_INDEX(virt));
    if (ptab == 0) ks.warn("physical address for virtual address %x is invalid", virt);

    return GET_PHYSICAL_ADDRESS(ptab->entries[(uint64_t)GET_TAB_INDEX(virt)]) + GET_PAGE_OFFSET(virt);
}

// *Return a new page table of 512 entries all set to not-present 
// @return the new page table pointer
PageTable* vmm_new_table() {
    PageTable* table = (PageTable*)get_mem_address(pmm_alloc());
    memory_set(table, 0, PHYSMEM_BLOCK_SIZE);

    return table;
}

// *Refresh paging by reloading the CR3 register
void volatile_fun vmm_refresh_paging() {
    __asm__("mov %%cr3, %%rax" : : );
    __asm__("mov %%rax, %%cr3" : : );
}

// *Check if a page table is free in each of its entries
// @param table the page table to check
// @return true if the page table is free, false otherwise
bool vmm_is_table_free(PageTable* table) {
    for (int i = 0; i < 512; i++) {
        if (vmm_get_entry(table, i) != 0) return false;
    }

    return true;
}

// *Free a physical memory block if the page pointing to it is free
// @param table the table to check if is free
// @param parent the parent page of the table
// @param parent_index the index of [table] in the [parent] page
void vmm_free_if_necessary_table(PageTable* table, PageTable* parent, uint64_t parent_index) {
    if (vmm_is_table_free(table)) {
        pmm_free(GET_PHYSICAL_ADDRESS(parent->entries[parent_index]));
        page_clear_bit(vmm_get_entry(table, parent_index), PRESENT_BIT_OFFSET);
    }
}

// *Free all the tables in the pml4 table where the given address is free
// @param table the pml4 table to check the address in
// @param address the address unmapped from the pml4 table
void vmm_free_if_necessary_tables(PageTable* table, uint64_t unmapped_addr) {
    uint64_t pl4_entry = GET_PL4_INDEX(unmapped_addr);
    uint64_t dpt_entry = GET_DPT_INDEX(unmapped_addr);
    uint64_t pd_entry = GET_DIR_INDEX(unmapped_addr);
    
    PageTable *pdpt, *pd, *pt;
    pdpt = vmm_get_or_create_entry(table, pl4_entry, true, true);
    pd = vmm_get_or_create_entry(pdpt, dpt_entry, true, true);
    pt = vmm_get_or_create_entry(pd, pd_entry, true, true);

    vmm_free_if_necessary_table(pt, pd, pd_entry);
    vmm_free_if_necessary_table(pd, pdpt, dpt_entry);
    vmm_free_if_necessary_table(pdpt, table, pl4_entry);
}

// --- Special mappings -------------------------

// *Map the physical regions stored in the physical manager to the given page
// @param phys the physical memory manager
// @param page the page to map the regions into
void vmm_map_physical_regions(PageTable* page) {
    for (int ind = 0; ind < pmm.regions_count; ind++) {
        if (pmm.regions[ind].type == MEMORY_REGION_USABLE ||
            pmm.regions[ind].type == MEMORY_REGION_INVALID ||
            pmm.regions[ind].type == MEMORY_REGION_FRAMEBUFFER) continue;

        uint64_t aligned_base = pmm.regions[ind].base - pmm.regions[ind].base % PAGE_SIZE;
        uint64_t aligned_size = ((pmm.regions[ind].size / PAGE_SIZE) + 1) * PAGE_SIZE;

        ks.dbg("region is %i bytes long, needing %i pages. Aligned base is %x, aligned length is %x", 
            pmm.regions[ind].size, pmm.regions[ind].size / PAGE_SIZE + 1, aligned_base, aligned_size);

        for (int i = 0; i * PAGE_SIZE < aligned_size; i++) {
            uintptr_t addr = aligned_base + i * PAGE_SIZE;
            vmm_map_page_impl(page, addr, addr, (PageProperties){true, false});
            vmm_map_page_impl(page, addr, get_mem_address(addr), (PageProperties){true, false});
            if (pmm.regions[ind].type == MEMORY_REGION_KERNEL) 
                vmm_map_page_impl(page, addr, get_kern_address(addr), (PageProperties){true, false});
        } 
    }
}

void volatile_fun vmm_map_kernel_region(struct memory_physical* phys, PageTable* page) {
    for (int ind = 0; ind < phys->regions_count; ind++) {
        if (phys->regions[ind].type != MEMORY_REGION_KERNEL) continue;

        uint64_t aligned_base = phys->regions[ind].base - phys->regions[ind].base % PAGE_SIZE;
        uint64_t aligned_size = ((phys->regions[ind].size / PAGE_SIZE) + 1) * PAGE_SIZE;

        for (int i = 0; i * PAGE_SIZE < aligned_size; i++) {
            uint64_t addr = aligned_base + i * PAGE_SIZE;
            vmm_map_page_impl(page, addr, addr, (PageProperties){true, false});
            vmm_map_page_impl(page, addr, get_kern_address(addr), (PageProperties){true, false});
        } 
    }
}

// *Identity map a region of physical memory
// @param table the PageTable pointer to the table to map addresses to
// @param base the base address of the memory region
// @param size the size of the memory region 
void volatile_fun vmm_identity_map_region(PageTable* table, uintptr_t base, size_t size) {
    for (int i = 0; i * PAGE_SIZE < size; i++) {
        uint64_t addr = base + i * PAGE_SIZE;
        vmm_map_page_impl(table, get_rmem_address(addr), addr, (PageProperties){true, false});
    }
}

// --- Mapping and unmapping --------------------

void vmm_map_page_impl(PageTable* table_addr, uintptr_t phys_addr, uintptr_t virt_addr, PageProperties prop) {
    PagingPath path = GetPagingPath(virt_addr);

    PageTable *pdpt, *pd, *pt;
    pdpt = vmm_get_or_create_entry(table_addr, path.pl4, true, false);
    pd = vmm_get_or_create_entry(pdpt, path.dpt, true, false);
    pt = vmm_get_or_create_entry(pd, path.pd, true, false);

    pt->entries[path.pt] = page_create(phys_addr, prop.writable, prop.user);
}

bool vmm_unmap_page_impl(PageTable* table_addr, uintptr_t virt_addr) {
    PagingPath path = GetPagingPath(virt_addr);
    
    PageTable *pdpt, *pd, *pt;
    pdpt = vmm_get_or_create_entry(table_addr, path.pl4, true, true);
    pd = vmm_get_or_create_entry(pdpt, path.dpt, true, true);
    pt = vmm_get_or_create_entry(pd, path.pd, true, true);

    PageTable *to_free = vmm_get_entry(pt, path.pt);
    if (to_free == 0) return false;

    page_clear_bit(to_free, PRESENT_BIT_OFFSET);

    vmm_free_if_necessary_tables(table_addr, virt_addr);
    vmm_refresh_paging();
    return true;
}

// *Get the recurse link for the active page table
// @return the address of the active page table
static inline uintptr_t vmm_get_active_recurse_link() {
    return GetRecursiveAddress(RECURSE_ACTIVE, RECURSE_ACTIVE, RECURSE_ACTIVE, RECURSE_ACTIVE, 0);
}

// *Get the recurse link for the other page table
// @return the address of the other page table
static inline uintptr_t vmm_get_other_recurse_link() {
    return GetRecursiveAddress(RECURSE_ACTIVE, RECURSE_ACTIVE, RECURSE_ACTIVE, RECURSE_OTHER, 0);
}

// === PUBLIC FUNCTIONS =========================

void init_vmm() {
    ks.log("Initializing VMM...");
    
    vmm.address_size = get_physical_address_length();

    // prepare a pml4 table for the kernel address space
    PageTable* kernel_pml4 = vmm_new_table();
    ks.dbg("New pml4 created at %x", kernel_pml4);

    // identity map the first 2M of physical memory
    for (int i = 0; i < PHYSMEM_2MEGS / PAGE_SIZE; i++)
        vmm_map_page_impl(kernel_pml4, i*PAGE_SIZE, i*PAGE_SIZE, (PageProperties){true, false});

    // map the page itself in the 510st pml4 entry
    ks.dbg("Mapping page table inside itself: %x (%x) to %x", kernel_pml4, get_rmem_address((uintptr_t)kernel_pml4), RECURSE_PML4);
    kernel_pml4->entries[RECURSE_ACTIVE] = page_self(kernel_pml4);

    // map physical regions in the page
    vmm_map_physical_regions(kernel_pml4);

    // map the pmm map
    vmm_identity_map_region(kernel_pml4, (uintptr_t)pmm._map - (uintptr_t)pmm._map % PAGE_SIZE, ((pmm._map_size / PAGE_SIZE) + 1) * PAGE_SIZE);
    
    // give CR3 the kernel pml4 address
    ks.dbg("Preparing to load pml4...");
    get_bootstrap_cpu()->page_table = get_rmem_address((uintptr_t)kernel_pml4);
    write_cr3(get_rmem_address((uintptr_t)kernel_pml4));

    ks.log("VMM has been initialized.");
}

void init_vmm_on_ap(struct stivale2_smp_info* info) {
    ks.log("Initializing VMM on CPU #%u...", info->processor_id);

    // prepare a pml4 table for the kernel address space
    PageTable* kernel_pml4 = vmm_new_table();
    ks.dbg("New pml4 created at %x for CPU #%u", kernel_pml4, info->processor_id);
    
    // identity map the first 2M of physical memory
    for (int i = 0; i < PHYSMEM_2MEGS / PAGE_SIZE; i++)
        vmm_map_page_impl(kernel_pml4, i*PAGE_SIZE, i*PAGE_SIZE, (PageProperties){true, false});

    // map the page itself in the 510st pml4 entry
    ks.dbg("Mapping page table inside itself: %x (%x) to %x", kernel_pml4, get_rmem_address((uintptr_t)kernel_pml4), RECURSE_PML4);
    kernel_pml4->entries[RECURSE_ACTIVE] = page_self(kernel_pml4);
    
    // map kernel in the page
    vmm_map_physical_regions(kernel_pml4); 

    // map the cpu stack in the page
    uint64_t stack_base = (uint64_t)(info->target_stack-CPU_STACK_SIZE) - (uint64_t)(info->target_stack - CPU_STACK_SIZE) % PAGE_SIZE;
    uint64_t stack_size = ((CPU_STACK_SIZE / PAGE_SIZE) + 1) * PAGE_SIZE;
    ks.dbg("Mapping CPU stack starting %x, length %x", stack_base, stack_size);
    for (int i = 0; i * PAGE_SIZE < stack_size; i++) {
        uintptr_t addr = stack_base + i * PAGE_SIZE;
        vmm_map_page_impl(kernel_pml4, get_rmem_address(addr), addr, (PageProperties){true, false});
    }

    // map the pmm map
    vmm_identity_map_region(kernel_pml4, (uintptr_t)pmm._map - (uintptr_t)pmm._map % PAGE_SIZE, ((pmm._map_size / PAGE_SIZE) + 1) * PAGE_SIZE);
    
    // give CR3 the kernel pml4 address
    ks.dbg("Preparing to load pml4... %x %x", kernel_pml4, get_rmem_address((uintptr_t)kernel_pml4));
    get_cpu(info->processor_id)->page_table = get_rmem_address((uintptr_t)kernel_pml4);
    write_cr3(get_rmem_address((uintptr_t)kernel_pml4));
    ks.log("VMM has been initialized.");
}

// *Map a physical address to a virtual page address
// @param table the table to map the address into
// @param phys_addr the physical address to map the virtual address to
// @param virt_addr the virtual address to map the physical address to
// @param writable flag to indicate whether the newly created entry should be writable or not
// @param user flags to indicate whether the newly created entry should be accessible from userspace or not
void vmm_map_page(PageTable* table, uintptr_t phys_addr, uintptr_t virt_addr, bool writable, bool user) {
    if (read_cr3() == table || table == 0) {               //  cr3 is the given table physical address
        vmm_map_page_impl(vmm_get_active_recurse_link(), phys_addr, virt_addr, (PageProperties){writable, user});
    
    } else {
        vmm_map_page_impl(vmm_get_active_recurse_link(), table, RECURSE_PML4_OTHER, (PageProperties){true, false});
        ((PageTable*)RECURSE_PML4_OTHER)->entries[RECURSE_OTHER] = page_create(table, true, false);
        vmm_refresh_paging();

        vmm_map_page_impl(vmm_get_other_recurse_link(), phys_addr, virt_addr, (PageProperties){writable, user});

        ((PageTable*)RECURSE_PML4_OTHER)->entries[RECURSE_OTHER] = 0;
        vmm_unmap_page_impl(vmm_get_active_recurse_link(), RECURSE_PML4_OTHER);
    }

    vmm_refresh_paging();
}

// *Unmap a page given the table and a virtual address in the page
// @param table the pml4 table the virtual address resides in
// @param virt_addr the virtual address in the page
// @return true if the page was successfully unmapped, false otherwise
bool vmm_unmap_page(PageTable* table, uintptr_t virt_addr) {
    bool res = false;
    
    if (read_cr3() == table || table == 0) {   // if cr3 is the given table physical address
        res = vmm_unmap_page_impl(vmm_get_active_recurse_link(), virt_addr);

    } else {
        vmm_map_page_impl(vmm_get_active_recurse_link(), table, RECURSE_PML4_OTHER, (PageProperties){true, false});
        ((PageTable*)RECURSE_PML4_OTHER)->entries[RECURSE_OTHER] = page_create(table, true, false);
        vmm_refresh_paging();

        res = vmm_unmap_page_impl(vmm_get_other_recurse_link(), virt_addr);

        ((PageTable*)RECURSE_PML4_OTHER)->entries[RECURSE_OTHER] = 0;
        vmm_unmap_page_impl(vmm_get_active_recurse_link(), RECURSE_PML4_OTHER);
    }

    vmm_refresh_paging();
    return res;
}

// *Map a physical address to a virtual address. Works with either an offline page table or an active one
// @param table the table to map the address into. 0 if current
// @param blocks the number of blocks to be mapped
// @param writable flag to indicate whether the newly created entry should be writable or not
// @param user flags to indicate whether the newly created entry should be accessible from userspace or not
// @return the virtual address of the newly allocated memory
uintptr_t vmm_allocate_memory(PageTableEntry* table, size_t blocks, bool writable, bool user) {
    uintptr_t phys_addr = pmm_alloc_series(blocks);

    for (size_t i = 0; i < blocks; i++) 
        vmm_map_page(table, phys_addr + (i*PHYSMEM_BLOCK_SIZE), get_mem_address(phys_addr + (i*PHYSMEM_BLOCK_SIZE)), writable, user);  

    return get_mem_address(phys_addr);
}

// *Unmap a memory area given the virtual address and the blocks. Works with either an offline page table or an active one
// @param table the table to unmap the address from. 0 if current
// @param addr the virtual address to unmap
// @param blocks the number of blocks to be unmapped
bool vmm_free_memory(PageTableEntry* table, uintptr_t addr, size_t blocks) {
    uintptr_t phys_addr = vmm_virt_to_phys(addr);
    for (size_t i = 0; i < blocks; i++) vmm_unmap_page(table, addr + (i*PHYSMEM_BLOCK_SIZE));

    pmm_free_series(phys_addr, blocks);
    return true;
}

// *Map a MMIO physical address to itself. Works with either an offline page table or an active one
// @param mmio_addr the memory mapped IO address to map
// @param blocks the number of blocks to be mapped
// @return the mmio mapped address
uintptr_t vmm_map_mmio(uintptr_t mmio_addr, size_t blocks) {
    for (size_t i = 0; i < blocks; i++) 
        vmm_map_page(0, mmio_addr + (i*PHYSMEM_BLOCK_SIZE), get_mmio_address(mmio_addr + (i*PHYSMEM_BLOCK_SIZE)), true, false);
    
    return get_mmio_address(mmio_addr);
}

// === SPACE FUNCTIONS ==========================

#include <liballoc.h>

PageTable* volatile_fun NewPageTable() {
    PageTable* p = (PageTable*)vmm_allocate_memory(get_current_cpu()->page_table, 1, true, false);
    memory_set(p, 0, sizeof(PageTable));
    p->entries[RECURSE_ACTIVE] = page_create(p, true, false);

    vmm_map_kernel_region(&pmm, vmm_virt_to_phys(p));
    vmm_identity_map_region(p, (uint64_t)pmm._map - (uint64_t)pmm._map % PAGE_SIZE, ((pmm._map_size / PAGE_SIZE) + 1) * PAGE_SIZE);

    for (uint32_t i = 0; i < PAGE_ENTRIES; i++)
        ks.dbg("%x - page[%u]: %x",&p->entries[i], i, p->entries[i]);

    return p;
}

void DestroyPageTable(PageTable* page_table) {
    vmm_free_memory(get_current_cpu()->page_table, page_table, 1);
} 

void vmm_switch_space(PageTable* page_table) {
    write_cr3(page_table);
}
