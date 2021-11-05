#include "mem_phys.h"
#include "kernel/common/kservice.h"
#include "kernel/common/memory/memory.h"
#include "libs/libc/stdbool.h"
#include "libs/libc/size_t.h"

// === PRIVATE FUNCTIONS ========================

// *Set a bit in the memory bitmap
// @param bit the bit to set
void pmm_map_set(int bit) {
    pmm._map[bit / 32] |= (1 << (bit % 32));
}

// *Reset a bit in the memory bitmap
// @param bit the bit to reset
void pmm_map_unset(int bit) {
    pmm._map[bit / 32] &= ~ (1 << (bit % 32));
}

// *Get the value of the memory bitmap at the position [bit]
// @param bit the bit to get the value from
// @return true if the bit is set, false otherwise
bool pmm_map_get(int bit) {
    return (pmm._map[bit / 32] & (1 << (bit % 32))) == 0 ? false : true;
}

// *Find the first free slot in the memory, and return it
// @return the number of the bit representing first free slot in the memory
int pmm_map_first_free() {
	for (uint32_t i=0; i < pmm.total_blocks / 32; i++)
		if (pmm._map[i] != 0xffffffff)
			for (int j=0; j<32; j++) {		//! test each bit in the dword
 
				int bit = 1 << j;
				if (! (pmm._map[i] & bit) )
					return i*4*8+j;
			}
 
	return -1;
}

// *Find the first free series of slots in the memory, and return the first of them 
// @param size the size of the series to find
// @return the first free bit of the series
int pmm_map_first_free_series(size_t size) {
    if (size==0)
		return -1;

	if (size==1)
		return pmm_map_first_free();

	for (uint32_t i=0; i < pmm.total_blocks/32; i++)
		if (pmm._map[i] != 0xffffffff)
			for (int j=0; j<32; j++) {	//! test each bit in the dword

				int bit = 1<<j;
				if (! (pmm._map[i] & bit) ) {

					int startingBit = i*32;
					startingBit+=bit;		//get the free bit in the dword at index i

					uint32_t free=0; //loop through each bit to see if its enough space
					for (uint32_t count=0; count<=size;count++) {

						if (! pmm_map_get (startingBit+count) )
							free++;	// this bit is clear (free frame)

						if (free==size)
							return i*4*8+j; //free count==size needed; return index
					}
				}
			}

	return -1;
}

// *Mark a region starting at [base_addr] of size [size] as free
// @param base_addr the base address of the region to be marked as free
// @param size the size of the region to be marked as free
void pmm_mark_region_free(uint64_t base_addr, size_t size) {
	int align = base_addr / PHYSMEM_BLOCK_SIZE;
	int blocks = size / PHYSMEM_BLOCK_SIZE;
 
	for (; blocks>0; blocks--) {
		pmm_map_unset(align++);
		pmm.used_blocks--;
	}
 
	pmm_map_set(0);	//first block is always set. This insures allocs cant be 0
}

// *Mark a region starting at [base_addr] of size [size] as allocated
// @param base_addr the base address of the region to be marked as allocated
// @param size the size of the region to be marked as allocated
void pmm_mark_region_used(uint64_t base_addr, size_t size) {
	int align = base_addr / PHYSMEM_BLOCK_SIZE;
	int blocks = size / PHYSMEM_BLOCK_SIZE;
 
	for (; blocks>0; blocks--) {
		pmm_map_set(align++);
		pmm.used_blocks++;
	}
}

// === PUBLIC FUNCTIONS =========================

// *Initialize the physical memory manager
// @param entries a valid array of memory_physical_region entries
// @param size the size of the array of memory_physical_region
void init_pmm(struct memory_physical_region *entries, uint32_t size) {    
    ks.dbg("Initializing PMM...");
    
    pmm.regions = entries;
    pmm.total_memory = entries[size-1].limit - entries[0].base;
    pmm.total_blocks = pmm.total_memory / PHYSMEM_BLOCK_SIZE;
    pmm.usable_memory = 0;
	pmm._map = 0;
	pmm._map_size = pmm.total_blocks / 8;

    for (int i = 0; i < size; i++) {
        struct memory_physical_region entry = entries[i];

		ks.dbg("%i: base %x length %u type %d", i, entry.base, entry.size, entry.type);
        
		// find the first usable region and set it as the base address of the memory bitmap
        if (entry.type == MEMORY_REGION_USABLE && pmm._map == 0 && entry.size >= pmm._map_size) {
			memory_set(entry.base, 0, pmm._map_size);
			pmm._map = entry.base;
		}
        
		// the usable_memory is the sum of all the usable memory regions
	    if (entry.type == MEMORY_REGION_USABLE) pmm.usable_memory += entry.size;
    }

	pmm.usable_blocks = pmm.usable_memory / PHYSMEM_BLOCK_SIZE;

	// mark the memory bitmap itself as used
	pmm_mark_region_used((uint64_t)pmm._map, pmm._map_size);

	// mark non-usable regions as used
	for (int i = 0; i < size; i++) {
		if (entries[i].type != MEMORY_REGION_USABLE) 
			pmm_mark_region_used(entries[i].base, entries[i].size);
	}

	ks.dbg("Memory map created at %x. Size is %u bytes", pmm._map, pmm._map_size);
	ks.dbg("Found %u bytes of usable memory. Preparing %u blocks", pmm.usable_memory, pmm.usable_blocks);
	ks.dbg("Used %u/%u blocks", pmm.used_blocks, pmm.usable_blocks);
	ks.dbg("PMM has been initialized.");
}

void print_memory_map(uint64_t base_addr, size_t size) {
	int align = base_addr / PHYSMEM_BLOCK_SIZE;
	int blocks = size / PHYSMEM_BLOCK_SIZE;

	ks.dbg("Memory map from %x", base_addr);
 
	for (; blocks>0; blocks--) 
		ks._put("%b", pmm_map_get(align++));

	ks._put("\n");
}
