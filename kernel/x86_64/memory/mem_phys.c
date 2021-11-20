#include "mem_phys.h"
#include "arch.h"
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

// *Find the first free slot in the memory starting from the specified block, and return it
// @param from_block the block to start searching from
// @return the number of the bit representing first free slot in the memory
int32_t pmm_map_first_free_starting_from(uint64_t from_block) {
	for (uint32_t i= from_block; i < pmm.total_blocks/32; i++)
		if (pmm._map[i] != 0xffffffff)
			for (int j=0; j<32; j++) {		//! test each bit in the dword
 				if (!(pmm._map[i] & 1 << j)) return i*4*8+j;
			}
 
	return -1;
}

// *Find the first free slot in the memory, and return it
// @return the number of the bit representing first free slot in the memory
int32_t pmm_map_first_free() {
	int32_t free_after_map = pmm_map_first_free_starting_from(((uint64_t)pmm._map + pmm._map_size) / PHYSMEM_BLOCK_SIZE);
	if (free_after_map != -1) return free_after_map;

	int32_t free_from_start = pmm_map_first_free_starting_from(0);
	if (free_from_start != -1) return free_from_start;
 
	return -1;
}

// *Find the first free series of slots in the memory starting from the specified block, and return the first of them 
// @param size the size of the series to find
// @param from_block the block to start searching from
// @return the first free bit of the series
int32_t pmm_map_first_free_series_starting_from(size_t size, uint32_t from_block) {
	for (uint32_t i=from_block; i < pmm.total_blocks/32; i++)
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

// *Find the first free series of slots in the memory, and return the first of them 
// @param size the size of the series to find
// @return the first free bit of the series
int32_t pmm_map_first_free_series(size_t size) {
    if (size==0) return -1;
	if (size==1) return pmm_map_first_free();

	int32_t free_after_map = pmm_map_first_free_series_starting_from(size, ((uint64_t)pmm._map + pmm._map_size) / PHYSMEM_BLOCK_SIZE);
	if (free_after_map != -1) return free_after_map;

	int32_t free_from_start = pmm_map_first_free_series_starting_from(size, 0);
	if (free_from_start != -1) return free_from_start;

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

void pmm_print_memory_map(uint64_t base_addr, size_t size) {
	int align = base_addr / PHYSMEM_BLOCK_SIZE;

	ks.dbg("Memory map from %x", base_addr);
 	ks._put("%x\t", base_addr);
	for (int block = 0; block < size; block++) {
		if (block % 64 == 0 && block != 0) ks._put("\n%x\t%b", base_addr+block*PHYSMEM_BLOCK_SIZE, pmm_map_get(align+block));
		else if (block % 32 == 0 && block != 0) ks._put("\t%b", pmm_map_get(align+block));
		else ks._put("%b", pmm_map_get(align+block));
	}

	ks._put("\n");
}

// === PUBLIC FUNCTIONS =========================

// *Initialize the physical memory manager
// @param entries a valid array of memory_physical_region entries
// @param size the size of the array of memory_physical_region
void init_pmm(struct memory_physical_region *entries, uint32_t size) {
    ks.log("Initializing PMM...");
    
    pmm.regions = entries;
	pmm.regions_count = size;
    pmm.total_memory = entries[size-1].limit - entries[0].base - pmm_get_region_by_type(MEMORY_REGION_FRAMEBUFFER).size;
    pmm.total_blocks = pmm.total_memory / PHYSMEM_BLOCK_SIZE;
    pmm.usable_memory = 0;
	pmm._map = 0;
	pmm._map_size = pmm.total_blocks / 8;

    for (int i = 0; i < size; i++) {
        struct memory_physical_region entry = entries[i];

		ks.dbg("%i: base %x length %u type %d", i, entry.base, entry.size, entry.type);
        
		// find the first usable region and set it as the base address of the memory bitmap
        if (entry.type == MEMORY_REGION_USABLE && pmm._map == 0 && entry.size >= pmm._map_size && entry.base >= PHYSMEM_2MEGS) {
			pmm._map = get_mem_address(entry.base);
			memory_set(pmm._map, 0, pmm._map_size);
		}
        
		// the usable_memory is the sum of all the usable memory regions
	    if (entry.type == MEMORY_REGION_USABLE) pmm.usable_memory += entry.size;
    }

	pmm.usable_blocks = pmm.usable_memory / PHYSMEM_BLOCK_SIZE;
	// if ((uint64_t)pmm._map != 0) pmm_mark_region_used(0, pmm._map_size - (uint64_t)pmm._map); 

	// mark the memory bitmap itself as used
	pmm_mark_region_used(get_rmem_address((uint64_t)pmm._map), pmm._map_size);

	// mark non-usable regions as used
	for (int i = 0; i < size; i++) {
		if (entries[i].type != MEMORY_REGION_USABLE) 
			pmm_mark_region_used(entries[i].base, entries[i].size);
	}

	ks.dbg("Memory map created at %x. Size is %u bytes", pmm._map, pmm._map_size);
	ks.log("Found %u bytes of usable memory. Preparing %u blocks", pmm.usable_memory, pmm.usable_blocks);
	ks.dbg("Used %u/%u blocks", pmm.used_blocks, pmm.usable_blocks);
	ks.log("PMM has been initialized.");
}

// *Allocate a physical memory block and return the physical address of the assigned region
// @return the physical address of the assigned block
uintptr_t pmm_alloc() {
	if (pmm.used_blocks >= pmm.usable_blocks) ks.panic("Out of physical memory!");
	
	uint32_t block = pmm_map_first_free();
	if (block == -1) ks.panic("Out of physical memory!");
	
	pmm_map_set(block);
	pmm.used_blocks++;

	ks.dbg("pmm_alloc() : first_free_block: %u return_address: %x", block, (uintptr_t)(block*PHYSMEM_BLOCK_SIZE));

	return (uintptr_t)(block*PHYSMEM_BLOCK_SIZE);
}

// *Free a physical memory block
// @param addr the address of the physical memory block to free
void pmm_free(uintptr_t addr) {
	uint32_t p = (uint32_t)addr;
	uint32_t block = p / PHYSMEM_BLOCK_SIZE;

	pmm_map_unset(block);
	pmm.used_blocks--;
}

// *Allocate a series physical memory blocks and return the physical address of the assigned region
// @param size the number of physical memory blocks to allocate
// @return the physical address of the assigned block
uintptr_t pmm_alloc_series(size_t size) {
	if (pmm.used_blocks + size >= pmm.usable_blocks) ks.panic("Out of physical memory!");

	uint32_t block = pmm_map_first_free_series(size);
	if (block == -1) ks.panic("Out of physical memory!");

	for (uint32_t i=0; i<size; i++) pmm_map_set(block+i);
	pmm.used_blocks += size;

	return (uintptr_t)(block*PHYSMEM_BLOCK_SIZE);
}

// *Free a series of physical memory block
// @param size the number of physical memory blocks to free
// @param addr the address of the physical memory block to free
void pmm_free_series(uintptr_t addr, size_t size) {
	uint32_t p = (uint32_t)addr;
	uint32_t block = p / PHYSMEM_BLOCK_SIZE;

	for (uint32_t i=0; i<size; i++) pmm_map_unset(block+i);
	pmm.used_blocks -= size;
}

// *Get the base address of a memory region given the type. Only return the first region found
// @param type the type of memory region to get
// @return the base address of the memory region, 0 if not found 
struct memory_physical_region pmm_get_region_by_type(memory_physical_region_type type) {
	for (uint32_t i = 0; i < pmm.regions_count; i++) {
		if (pmm.regions[i].type != type) continue;
		return (struct memory_physical_region)pmm.regions[i];
	}

	return pmm.regions[0];
}
