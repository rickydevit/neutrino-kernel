#pragma once
#include <stdbool.h>
#include <stdint.h>

#define PAGE_SIZE       0x1000
#define RECURSE_ACTIVE  510UL
#define RECURSE_OTHER   509UL
#define RECURSE_PML4        (0xffffff0000000000UL)
#define RECURSE_PML4_OTHER  (0xfffffe8000000000UL)
#define PAGE_ENTRIES 512

#define GetRecursiveAddress(gate, p3, p2, p1, offset) (PageTableEntry*)((0xffffUL<<48) | (gate<<39) | (p3<<30) | (p2<<21) | (p1<<12)) + offset

#define PRESENT_BIT_OFFSET      0b1
#define WRITABLE_BIT_OFFSET     0b10
#define USERSPACE_BIT_OFFSET    0b100
#define CACHE_DISABLE_BIT_OFFSET 0b1000
#define ACCESSED_BIT_OFFSET     0b100000
#define DIRTY_BIT_OFFSET        0b1000000
#define HUGE_BIT_OFFSET         0b10000000
#define NO_EXECUTE_BIT_OFFSET   0x8000000000000000
#define ADDRESS_MASK            0xfffffffffffff000

#define IS_PRESENT(x)   (x & PRESENT_BIT_OFFSET)
#define IS_WRITABLE(x)  ((x & WRITABLE_BIT_OFFSET) >> 1)
#define IS_USERSPACE(x) ((x & USERSPACE_BIT_OFFSET) >> 2)
#define IS_ACCESSED(x)  ((x & ACCESSED_BIT_OFFSET) >> 4)
#define IS_DIRTY(x)     ((x & DIRTY_BIT_OFFSET) >> 5)
#define IS_HUGE(x)      ((x & HUGE_BIT_OFFSET) >> 7)
#define GET_PHYSICAL_ADDRESS(x) (x & ~0xfff)

#define GET_PL4_INDEX(x)    ((x & 0xff8000000000) >> 39)
#define GET_DPT_INDEX(x)    ((x & 0x007fc0000000) >> 30)
#define GET_DIR_INDEX(x)    ((x & 0x00003fe00000) >> 21)
#define GET_TAB_INDEX(x)    ((x & 0x0000001ff000) >> 12)
#define GET_PAGE_OFFSET(x)  (x & 0x000000000fff)

typedef uint64_t PageTableEntry;

typedef struct __page_table {
    PageTableEntry entries[PAGE_ENTRIES];
} PageTable;

// PagingPath

typedef struct __paging_path {
    uint64_t pl4;
    uint64_t dpt;
    uint64_t pd;
    uint64_t pt;
} PagingPath;

#define GetPagingPath(virt_addr) (PagingPath){GET_PL4_INDEX(virt_addr), GET_DPT_INDEX(virt_addr), GET_DIR_INDEX(virt_addr), GET_TAB_INDEX(virt_addr)}
#define GetAddress(paging_path, page)  (uintptr_t)((0xffffUL<<48) | (paging_path.pl4<<39) | (paging_path.dpt<<30) | (paging_path.pd<<21) | (paging_path.pt<<12)) + page

// PageProperties

typedef struct __page_properties {
    bool writable;
    bool user;
    bool cache_disable;
} PageProperties;

#define PageKernelWrite (PageProperties){true, false, false}
#define PageUserWrite   (PageProperties){true, true, false}

bool is_paging_enabled();
void disable_paging();
uint64_t read_cr3();
void write_cr3(uint64_t value);

inline void page_set_bit(PageTableEntry* page, uint64_t offset) { *page |= (offset); }
inline void page_clear_bit(PageTableEntry* page, uint64_t offset) { *page &= ~(offset); }

PageTableEntry page_create(uint64_t addr, PageProperties prop);
PageTableEntry page_self(PageTable* table);