#pragma once
#include "libs/libc/stdbool.h"

#define PAGE_SIZE 0x1000

#define PAGE_PL4_ENTRIES 512
#define PAGE_DPT_ENTRIES 512
#define PAGE_DIR_ENTRIES 512
#define PAGE_TAB_ENTRIES 512

#define PRESENT_BIT_OFFSET      0b1
#define WRITABLE_BIT_OFFSET     0b10
#define USERSPACE_BIT_OFFSET    0b100
#define ACCESSED_BIT_OFFSET     0b100000
#define DIRTY_BIT_OFFSET        0b1000000
#define HUGE_BIT_OFFSET         0b10000000
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

typedef uint64_t page_table_e;

typedef struct _page_table {
    page_table_e entries[PAGE_TAB_ENTRIES];
} page_table;

bool is_paging_enabled();
void disable_paging();
uint64_t read_cr3();
void write_cr3(uint64_t value);

inline void page_set_bit(page_table_e* page, uint64_t offset) { *page |= (offset); }
inline void page_clear_bit(page_table_e* page, uint64_t offset) { *page &= ~(offset); }

page_table_e page_create(uint64_t addr, bool writable, bool user);
