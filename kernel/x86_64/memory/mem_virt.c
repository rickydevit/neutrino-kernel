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

void vmm_map_page_impl(PageTable* table_addr, uintptr_t phys_addr, uintptr_t virt_addr, PageProperties prop);

// -- Utilities ---------------------------------

void unoptimized vmm_reload_tlb(uintptr_t addr) {
       asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

// *Refresh paging by reloading the CR3 register
void unoptimized vmm_reload_cr3() {
    asm volatile("mov %%cr3, %%rax" : : );
    asm volatile("mov %%rax, %%cr3" : : );
}

// *Get the recurse link for the active page table
// @return the address of the active page table
static inline uintptr_t vmm_get_active_recurse_link() {
    return (uintptr_t)GetRecursiveAddress(RECURSE_ACTIVE, RECURSE_ACTIVE, RECURSE_ACTIVE, RECURSE_ACTIVE, 0);
}

// *Get the recurse link for the other page table
// @return the address of the other page table
static inline uintptr_t vmm_get_other_recurse_link() {
    return (uintptr_t)GetRecursiveAddress(RECURSE_ACTIVE, RECURSE_ACTIVE, RECURSE_ACTIVE, RECURSE_OTHER, 0);
}


// *Get a page entry in the given page table
// @param table the page table to search the entry in
// @param entry the entry to be found
// @return a pointer to the page entry requested, or 0 if not found
PageTable* vmm_get_entry(PageTable* table, uint64_t entry) {
    if (IS_PRESENT(table->entries[entry])) 
        return (PageTable*)get_mem_address(GET_PHYSICAL_ADDRESS(table->entries[entry]));
    return 0;
}

// *Create a new page table entry in the given table at the given entry with the specified properties
// @param table the table to add the entry to
// @param entry the index of the entry to be added
// @param prop the properties of the page entry
// @return the address of the newly created entry
PageTable* unoptimized vmm_create_entry(PageTable* table, uint64_t entry, PageProperties prop) {
    PageTable* pt = (PageTable*)get_mem_address(pmm_alloc());
    table->entries[entry] = page_create(get_rmem_address((uintptr_t)pt), prop);
    vmm_reload_cr3();

    return pt;
}

// *Get or create the entry in the given page table. If the entry already exists in the page table, its address
// *is returned. Otherwise a new entry is created and returned.
// @param table the table to add the entry to
// @param entry the index of the entry to be added
// @param prop the properties of the page entry
// @return the address of the existing table or the newly created table
PageTable* unoptimized vmm_get_or_create_entry(PageTable* table, uint64_t entry, PageProperties prop) {
    if (IS_PRESENT(table->entries[entry])) {
        return (PageTable*)get_mem_address(GET_PHYSICAL_ADDRESS(table->entries[entry]));
    } else {
        return vmm_create_entry(table, entry, prop);
    }
}

// *Return the physical address given a virtual address
// @param virt the virtual address
// @return the physical address associated to the virtual address
uintptr_t vmm_virt_to_phys(PageTable* table, uintptr_t virt) {
    if (table == (PageTable*)read_cr3() || table == 0) {
        PagingPath path = GetPagingPath(virt);
        table = GetRecursiveAddress(RECURSE_ACTIVE, path.pl4, path.dpt, path.pd, 0);
        return GET_PHYSICAL_ADDRESS(table->entries[path.pt]) + GET_PAGE_OFFSET(virt);
    
    } else {
        PageTable* pdpt = vmm_get_entry(table, (uint64_t)GET_PL4_INDEX(virt));
        if (pdpt == 0) ks.warn("Physical address for virtual address %x is invalid", virt);
        
        PageTable* pdir = vmm_get_entry(pdpt, (uint64_t)GET_DPT_INDEX(virt));
        if (pdir == 0) ks.warn("Physical address for virtual address %x is invalid", virt);
    
        PageTable* ptab = vmm_get_entry(pdir, (uint64_t)GET_DIR_INDEX(virt));
        if (ptab == 0) ks.warn("physical address for virtual address %x is invalid", virt);

        return GET_PHYSICAL_ADDRESS(ptab->entries[(uint64_t)GET_TAB_INDEX(virt)]) + GET_PAGE_OFFSET(virt);
    }

    return nullptr;
}

// *Return a new page table of 512 entries all set to not-present 
// @return the new page table pointer
PageTable* vmm_new_table() {
    PageTable* table = (PageTable*)get_mem_address(pmm_alloc());
    memory_set((uint8_t*)table, 0, PHYSMEM_BLOCK_SIZE);

    return table;
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

// *Find a free table entry in the given table and return its index
// @param table the page table to check
// @return the index of the free table entry. -1 if not found
int64_t vmm_find_table_free(PageTable* table) {
    for (int i = 0; i < 512; i++) {
        if (vmm_get_entry(table, i) != 0) continue;
        else return i;
    }

    return -1;
}

int64_t vmm_find_table_free_series(PageTable* table, size_t size){
    for (int i = 0; i < 512; i++) {
        if (vmm_get_entry(table, i) != 0) continue;
        
        bool bad = false;
        for (int j = i+1; j < size; j++) {
            if (vmm_get_entry(table, j) != 0) bad = true;
        }

        if (bad) continue;
        else return i;
    }

    return -1;
}

PageTable* unoptimized vmm_get_table_address(PageTable* table_addr, uintptr_t virt_addr, uint16_t depth) {
    if (depth == 0) return table_addr;
    if (depth > 3) depth = 3;

    PagingPath path = GetPagingPath(virt_addr);
    PagingPath tpath = GetPagingPath((uintptr_t)table_addr);
    bool isRecursive = tpath.pl4 == RECURSE_ACTIVE || tpath.pl4 == RECURSE_OTHER;

    PageTable* pdpt;

    if (isRecursive) {
        pdpt = (PageTable*)GetRecursiveAddress(tpath.pl4, tpath.pl4, tpath.pt, path.pl4, 0);
        if (depth == 1) return (PageTable*)GetRecursiveAddress(tpath.pl4, tpath.pl4, tpath.pt, path.pl4, 0);
        vmm_get_or_create_entry((PageTable*)GetRecursiveAddress(tpath.pl4, tpath.pl4, tpath.pt, path.pl4, 0), path.dpt, PageKernelWrite);
        if (depth == 2) return (PageTable*)GetRecursiveAddress(tpath.pl4, tpath.pt, path.pl4, path.dpt, 0);
        vmm_get_or_create_entry((PageTable*)GetRecursiveAddress(tpath.pl4, tpath.pt, path.pl4, path.dpt, 0), path.pd, PageKernelWrite);
        if (depth == 3) return (PageTable*)GetRecursiveAddress(tpath.pt, path.pl4, path.dpt, path.pd, 0);
    } else {
        pdpt = vmm_get_or_create_entry(table_addr, path.pl4, PageKernelWrite);
        PageTable* pd = vmm_get_or_create_entry(pdpt, path.dpt, PageKernelWrite);
        if (depth == 2) return pd;
        else return vmm_get_or_create_entry(pd, path.pd, PageKernelWrite);
    }

    return (PageTable*)nullptr;
}

// *Free a physical memory block if the page pointing to it is free
// @param table the table to check if is free
// @param parent the parent page of the table
// @param parent_index the index of [table] in the [parent] page
void vmm_free_if_necessary_table(PageTable* table, PageTable* parent, uint64_t parent_index) {
    if (vmm_is_table_free(table)) {
        pmm_free(GET_PHYSICAL_ADDRESS(parent->entries[parent_index]));
        page_clear_bit((PageTableEntry*)vmm_get_entry(table, parent_index), PRESENT_BIT_OFFSET);
    }
}

// *Free all the tables in the pml4 table where the given address is free
// @param table the pml4 table to check the address in
// @param address the address unmapped from the pml4 table
void vmm_free_if_necessary_tables(PageTable* table, uint64_t unmapped_addr) {
    PagingPath path = GetPagingPath(unmapped_addr);
    
    PageTable *pdpt, *pd, *pt;
    pdpt = vmm_get_table_address(table, unmapped_addr, 1);
    pd = vmm_get_table_address(table, unmapped_addr, 2);
    pt = vmm_get_table_address(table, unmapped_addr, 3);

    vmm_free_if_necessary_table(pt, pd, path.pd);
    vmm_free_if_necessary_table(pd, pdpt, path.dpt);
    vmm_free_if_necessary_table(pdpt, table, path.pl4);
}

PageTable* unoptimized vmm_get_most_nested_table(PageTable* table_addr, uintptr_t virt_addr, PageProperties prop) {
    PagingPath path = GetPagingPath(virt_addr);
    PagingPath tpath = GetPagingPath((uintptr_t)table_addr);
    bool isRecursive = tpath.pl4 == RECURSE_ACTIVE || tpath.pl4 == RECURSE_OTHER;
    PageProperties su_prop = (PageProperties) {
        .cache_disable = prop.cache_disable,
        .user = prop.user,
        .writable = true
    };

    PageTable *pdpt, *pd, *pt;
    
    if (isRecursive) {
        vmm_get_or_create_entry((PageTable*)GetRecursiveAddress(tpath.pl4, tpath.pl4, tpath.pl4, tpath.pt, 0), path.pl4, su_prop);
        vmm_get_or_create_entry((PageTable*)GetRecursiveAddress(tpath.pl4, tpath.pl4, tpath.pt, path.pl4, 0), path.dpt, su_prop);
        vmm_get_or_create_entry((PageTable*)GetRecursiveAddress(tpath.pl4, tpath.pt, path.pl4, path.dpt, 0), path.pd, su_prop);
        pt = (PageTable*)GetRecursiveAddress(tpath.pt, path.pl4, path.dpt, path.pd, 0);
    } else {
        pdpt = vmm_get_or_create_entry(table_addr, path.pl4, su_prop);
        pd = vmm_get_or_create_entry(pdpt, path.dpt, su_prop);
        pt = vmm_get_or_create_entry(pd, path.pd, su_prop);
    }

    return pt;
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
            // vmm_map_page_impl(page, addr, addr, (PageProperties){true, false});
            vmm_map_page_impl(page, addr, get_mem_address(addr), (PageProperties){true, false});
            if (pmm.regions[ind].type == MEMORY_REGION_KERNEL) 
                vmm_map_page_impl(page, addr, get_kern_address(addr), (PageProperties){true, false});
        } 
    }
}

void unoptimized vmm_map_kernel_region(struct memory_physical* phys, PageTable* page) {
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

// --- Mapping and unmapping --------------------

void unoptimized vmm_map_page_impl(PageTable* table_addr, uintptr_t phys_addr, uintptr_t virt_addr, PageProperties prop) {
    PagingPath path = GetPagingPath(virt_addr);
    PageTable* pt = vmm_get_most_nested_table(table_addr, virt_addr, prop);

    pt->entries[path.pt] = page_create(phys_addr, prop);
}

bool vmm_unmap_page_impl(PageTable* table_addr, uintptr_t virt_addr) {
    PageTable* pt = vmm_get_most_nested_table(table_addr, virt_addr, PageKernelWrite);

    // PageTable* to_free = vmm_get_entry(pt, path.pt);
    if (pt == nullptr) return false;

    page_clear_bit((PageTableEntry*)pt, PRESENT_BIT_OFFSET);

    vmm_free_if_necessary_tables(table_addr, virt_addr); // todo: reimplement this
    vmm_reload_tlb(virt_addr);
    return true;
}

uintptr_t unoptimized vmm_find_free_heap_series(size_t size, uintptr_t heap_base) {
    PageTable* pl4_addr = GetRecursiveAddress(RECURSE_ACTIVE, RECURSE_ACTIVE, RECURSE_ACTIVE, RECURSE_ACTIVE, GET_PL4_INDEX(heap_base));
    for (int dpt = 0; dpt < 512; dpt++) {
            
        vmm_get_or_create_entry(pl4_addr, dpt, (PageProperties){true, false, true});
        PageTable* dpt_addr = GetRecursiveAddress(RECURSE_ACTIVE, RECURSE_ACTIVE, RECURSE_ACTIVE, GET_PL4_INDEX(heap_base), dpt);
        for (int pd = 0; pd < 512; pd++) {
        
            vmm_get_or_create_entry(dpt_addr, pd, (PageProperties){true, false, true});
            PageTable* pd_addr = GetRecursiveAddress(RECURSE_ACTIVE, RECURSE_ACTIVE, GET_PL4_INDEX(heap_base), dpt, pd);
            for (int pt = 0; pt < 512; pt++) {
                if (vmm_get_entry(pd_addr, pt) != 0) continue;
        
                vmm_get_or_create_entry(pd_addr, pt, (PageProperties){true, false, true});
                PageTable* pt_addr = GetRecursiveAddress(RECURSE_ACTIVE, GET_PL4_INDEX(heap_base), dpt, pd, pt);
                for (int page = 0; page < 512; page++) {
                    if (vmm_get_entry(pt_addr, page) != 0) continue;

                    bool bad = false;
                    for (int j = page+1; j < size; j++) {
                        if (vmm_get_entry(pd_addr, j) != 0) bad = true;
                    }

                    if (bad) continue;
                    PagingPath path = (PagingPath){GET_PL4_INDEX(heap_base), dpt, pd, page};
                    return GetAddress(path, page);
                }
            }
        }
    }

    ks.fatal(FatalError(OUT_OF_HEAP, "Out of Kernel heap!"));
    return nullptr;
}

// === PUBLIC FUNCTIONS =========================

void init_vmm() {
    ks.log("Initializing VMM...");
    
    vmm.address_size = get_physical_address_length();

    // prepare a pml4 table for the kernel address space
    PageTable* kernel_pml4 = vmm_new_table();
    ks.dbg("New pml4 created at %x", kernel_pml4);

    // map the page itself in the 510st pml4 entry
    ks.dbg("Mapping page table inside itself: %x (%x) to %x", kernel_pml4, get_rmem_address((uintptr_t)kernel_pml4), RECURSE_PML4);
    kernel_pml4->entries[RECURSE_ACTIVE] = page_self(kernel_pml4);

    // map physical regions in the page
    vmm_map_physical_regions(kernel_pml4);

    // map the physical memory bitmap 
    for (int i = 0; i * PAGE_SIZE < PHYSMEM_MAP_SIZE; i++) 
        vmm_map_page_impl(kernel_pml4, get_rmem_address(PHYSMEM_MAP_BASE + (i * PAGE_SIZE)), PHYSMEM_MAP_BASE + (i * PAGE_SIZE), (PageProperties){true, false});
    
    // give CR3 the kernel pml4 address
    ks.dbg("Preparing to load pml4...");
    get_bootstrap_cpu()->page_table = (PageTable*)get_rmem_address((uintptr_t)kernel_pml4);
    write_cr3(get_rmem_address((uintptr_t)kernel_pml4));

    ks.log("VMM has been initialized.");
}

void unoptimized init_vmm_on_ap(struct stivale2_smp_info* info) {
    ks.log("Initializing VMM on CPU #%u...", info->processor_id);

    // prepare a pml4 table for the kernel address space
    PageTable* kernel_pml4 = vmm_new_table();
    ks.dbg("New pml4 created at %x for CPU #%u", kernel_pml4, info->processor_id);
    
    // clone 256-511 entries
    ks.dbg("Cloning BSP page table...");
    for (uint32_t entry = 256; entry < PAGE_ENTRIES; entry++) 
        kernel_pml4->entries[entry] = ((PageTable*)get_mem_address((uintptr_t)get_bootstrap_cpu()->page_table))->entries[entry];

    // map the page itself in the 510st pml4 entry
    ks.dbg("Mapping page table inside itself: %x (%x) to %x", kernel_pml4, get_rmem_address((uintptr_t)kernel_pml4), RECURSE_PML4);
    kernel_pml4->entries[RECURSE_ACTIVE] = page_self(kernel_pml4);

    // clear mmio pages
    for (uint32_t i = 480; i < 484; i++)
        kernel_pml4->entries[i] = 0;

    // map the cpu stack in the page
    uint64_t stack_base = (uint64_t)(info->target_stack-CPU_STACK_SIZE) - (uint64_t)(info->target_stack - CPU_STACK_SIZE) % PAGE_SIZE;
    uint64_t stack_size = ((CPU_STACK_SIZE / PAGE_SIZE)) * PAGE_SIZE;
    ks.dbg("Mapping CPU stack starting %x, length %x", stack_base, stack_size);
    for (int i = 0; i * PAGE_SIZE < stack_size; i++) {
        uintptr_t addr = stack_base + i * PAGE_SIZE;
        vmm_map_page_impl(kernel_pml4, get_rmem_address(addr), addr, (PageProperties){true, false});
    }
    
    // give CR3 the kernel pml4 address
    ks.dbg("Preparing to load pml4... %x %x", kernel_pml4, get_rmem_address((uintptr_t)kernel_pml4));
    get_cpu(info->processor_id)->page_table = (PageTable*)get_rmem_address((uintptr_t)kernel_pml4);
    write_cr3(get_rmem_address((uintptr_t)kernel_pml4));
    ks.log("VMM has been initialized.");
}

// *Map a physical address to a virtual page address
// @param table the table to map the address into
// @param phys_addr the physical address to map the virtual address to
// @param virt_addr the virtual address to map the physical address to
// @param prop the properties of the page entry
void unoptimized vmm_map_page(PageTable* table, uintptr_t phys_addr, uintptr_t virt_addr, PageProperties prop) {
    if (read_cr3() == (uintptr_t)table || table == 0) {               //  cr3 is the given table physical address
        vmm_map_page_impl((PageTable*)vmm_get_active_recurse_link(), phys_addr, virt_addr, prop);
    
    } else {
        ((PageTable*)vmm_get_active_recurse_link())->entries[RECURSE_OTHER] = page_create((uint64_t)table, prop);
        vmm_reload_tlb(vmm_get_other_recurse_link());

        vmm_map_page_impl((PageTable*)vmm_get_other_recurse_link(), phys_addr, virt_addr, prop);

        ((PageTable*)vmm_get_active_recurse_link())->entries[RECURSE_OTHER] = 0;
    }

    vmm_reload_tlb(virt_addr);
}

// *Unmap a page given the table and a virtual address in the page
// @param table the pml4 table the virtual address resides in
// @param virt_addr the virtual address in the page
// @return true if the page was successfully unmapped, false otherwise
bool vmm_unmap_page(PageTable* table, uintptr_t virt_addr) {
    bool res = false;
    
    if (read_cr3() == (uintptr_t)table || table == 0) {   // if cr3 is the given table physical address
        res = vmm_unmap_page_impl((PageTable*)vmm_get_active_recurse_link(), virt_addr);

    } else {
        ((PageTable*)vmm_get_active_recurse_link())->entries[RECURSE_OTHER] = page_create((uint64_t)table, PageKernelWrite);
        vmm_reload_tlb(RECURSE_PML4_OTHER);

        res = vmm_unmap_page_impl((PageTable*)vmm_get_other_recurse_link(), virt_addr);

        ((PageTable*)vmm_get_active_recurse_link())->entries[RECURSE_OTHER] = 0;
    }

    vmm_reload_tlb(virt_addr);
    return res;
}

// *Map a physical address to a virtual address. Works with either an offline page table or an active one
// @param table the table to map the address into. 0 if current
// @param blocks the number of blocks to be mapped
// @param prop the properties of the page entry
// @return the virtual address of the newly allocated memory
uintptr_t vmm_allocate_memory(PageTable* table, size_t blocks, PageProperties prop) {
    uintptr_t phys_addr = pmm_alloc_series(blocks);

    for (size_t i = 0; i < blocks; i++) 
        vmm_map_page(table, phys_addr + (i*PHYSMEM_BLOCK_SIZE), get_mem_address(phys_addr + (i*PHYSMEM_BLOCK_SIZE)), prop);  

    return get_mem_address(phys_addr);
}

uintptr_t unoptimized vmm_allocate_heap(size_t blocks, bool user) {
    uintptr_t heap_base = (user ? USER_HEAP_OFFSET : HEAP_OFFSET);
    uintptr_t phys_addr = pmm_alloc_series(blocks);
    uintptr_t virt_addr = vmm_find_free_heap_series(blocks, heap_base);

    for (size_t i = 0; i < blocks; i++) 
        vmm_map_page(0, phys_addr + (i*PHYSMEM_BLOCK_SIZE), virt_addr + (i*PHYSMEM_BLOCK_SIZE), (PageProperties){true, false, true});
    
    if (get_cpu_count() > 1) {
        for (size_t i = 0; i < get_cpu_count(); i++) {
            if (get_current_cpu()->id == i) continue;
            
            ((PageTable*)vmm_get_active_recurse_link())->entries[RECURSE_OTHER] = page_create((uint64_t)get_cpu(i)->page_table, (PageProperties){true, false, true});
            vmm_reload_tlb(vmm_get_other_recurse_link());

            ((PageTable*)vmm_get_other_recurse_link())->entries[GET_PL4_INDEX(heap_base)] = ((PageTable*)vmm_get_active_recurse_link())->entries[GET_PL4_INDEX(heap_base)];
        }

        ((PageTable*)vmm_get_active_recurse_link())->entries[RECURSE_OTHER] = 0;
        vmm_reload_tlb(vmm_get_other_recurse_link());
    } 

    return virt_addr;
}

// *Unmap a memory area given the virtual address and the blocks. Works with either an offline page table or an active one
// @param table the table to unmap the address from. 0 if current
// @param addr the virtual address to unmap
// @param blocks the number of blocks to be unmapped
bool vmm_free_memory(PageTable* table, uintptr_t addr, size_t blocks) {
    uintptr_t phys_addr = vmm_virt_to_phys(table, addr);
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
        vmm_map_page(0, mmio_addr + (i*PHYSMEM_BLOCK_SIZE), get_mmio_address(mmio_addr + (i*PHYSMEM_BLOCK_SIZE)), PageKernelWrite);
    
    return get_mmio_address(mmio_addr);
}

// === SPACE FUNCTIONS ==========================

#include <liballoc.h>

PageTable* unoptimized NewPageTable() {
    PageTable* p = (PageTable*)vmm_allocate_memory(get_current_cpu()->page_table, 1, PageKernelWrite);
    memory_set((uint8_t*)p, 0, PAGE_SIZE);

    // clone 256-511 entries
    for (uint32_t entry = 256; entry < PAGE_ENTRIES; entry++) 
        p->entries[entry] = ((PageTable*)vmm_get_active_recurse_link())->entries[entry];

    // set active
    p->entries[RECURSE_ACTIVE] = page_self(p);
    return (PageTable*)get_rmem_address((uintptr_t)p);
}

void unoptimized DestroyPageTable(PageTable* page_table) {
    write_cr3((uintptr_t)get_current_cpu()->page_table);
    vmm_free_memory(get_current_cpu()->page_table, get_mem_address((uintptr_t)page_table), 1);
} 

void unoptimized vmm_switch_space(PageTable* page_table) {
    write_cr3((uint64_t)page_table);
    vmm_reload_cr3();
}
