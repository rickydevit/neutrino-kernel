#include "mem_phys.h"
#include "arch.h"
#include "kernel/common/kservice.h"
#include "kernel/common/memory/memory.h"
#include "stdbool.h"
#include "size_t.h"
#include <neutrino/macros.h>

// === PRIVATE FUNCTIONS ========================

// --- Bitmap functions -------------------------

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
BlockState unoptimized pmm_map_get(int bit) {
    return (pmm._map[bit / 32] & (1 << (bit % 32)));
}

// *Find the first free slot in the memory starting from the specified block, and return it
// @param from_block the block to start searching from
// @return the number of the bit representing first free slot in the memory
BlockPosition pmm_map_first_free_starting_from(BlockPosition from_block) {
	for (BlockPosition i= from_block; i < pmm.total_blocks/32; i++)
		if (pmm._map[i] != 0xffffffff)
			for (int j=0; j<32; j++) {		//! test each bit in the dword
 				if (!(pmm._map[i] & 1 << j)) return i*4*8+j;
			}
 
	return BLOCKPOSITION_INVALID;
}

// *Find the first free slot in the memory, and return it
// @return the number of the bit representing first free slot in the memory
BlockPosition pmm_map_first_free() {
	BlockPosition free_from_start = pmm_map_first_free_starting_from(0);
	if (free_from_start != -1) return free_from_start;
 
	return BLOCKPOSITION_INVALID;
}

// *Find the first free series of slots in the memory starting from the specified block, and return the first of them 
// @param size the size of the series to find
// @param from_block the block to start searching from
// @return the first free bit of the series
BlockPosition pmm_map_first_free_series_starting_from(size_t size, BlockPosition from_block) {
	for (BlockPosition i=from_block; i < pmm.total_blocks/32; i++)
		if (pmm._map[i] != 0xffffffff)
			for (int j=0; j<32; j++) {	//! test each bit in the dword

				int bit = 1<<j;
				if (!(pmm._map[i] & bit)) {

					uint32_t startingBit = i*32;
					startingBit+=j;		//get the free bit in the dword at index i

					uint32_t free=0; //loop through each bit to see if its enough space
					for (BlockPosition count=0; count<=size; count++) {

						if (!pmm_map_get(startingBit+count))
							free++;	// this bit is clear (free frame)

						if (free==size)
							return i*4*8+j; //free count==size needed; return index
					}
				}
			}

	return 	BLOCKPOSITION_INVALID;
}

// *Find the first free series of slots in the memory, and return the first of them 
// @param size the size of the series to find
// @return the first free bit of the series
BlockPosition pmm_map_first_free_series(size_t size) {
    if (size==0) return -1;
	if (size==1) return pmm_map_first_free();

	BlockPosition free_after_map = pmm_map_first_free_series_starting_from(size, get_rmem_address(((uint64_t)pmm._map) + pmm._map_size) / PHYSMEM_BLOCK_SIZE);
	if (free_after_map != BLOCKPOSITION_INVALID) return free_after_map;

	BlockPosition free_from_start = pmm_map_first_free_series_starting_from(size, 0);
	if (free_from_start != BLOCKPOSITION_INVALID) return free_from_start;

	return -1;
}

void pmm_update_blocks(uint32_t used_block_increment) {
	pmm.used_blocks += used_block_increment;
	pmm.usable_blocks -= used_block_increment;
}

// --- Region functions -------------------------

// *Mark a region starting at [base_addr] of size [size] as free
// @param base_addr the base address of the region to be marked as free
// @param size the size of the region to be marked as free
void pmm_mark_region_free(uint64_t base_addr, size_t size) {
	int align = Align(base_addr);
	int blocks = Align(size);
 
	for (; blocks>0; blocks--) {
		pmm_map_unset(align++);
		pmm_update_blocks(-1);
	}
}

// *Mark a region starting at [base_addr] of size [size] as allocated
// @param base_addr the base address of the region to be marked as allocated
// @param size the size of the region to be marked as allocated
void pmm_mark_region_used(uint64_t base_addr, size_t size) {
	int align = Align(base_addr);
	int blocks = Align(size);
 
	for (; blocks>0; blocks--) {
		pmm_map_set(align++);
		pmm_update_blocks(+1);
	}
}

char* pmm_region_type_string(memory_physical_region_type type) {
	switch (type) {
		case MEMORY_REGION_USABLE: return "USABLE";
		case MEMORY_REGION_FRAMEBUFFER: return "FRAMEBUFFER";
		case MEMORY_REGION_ACPI_RCLM: return "ACPI_RCLM";
		case MEMORY_REGION_ACPI_RSVD: return "ACPI_RSVD";
		case MEMORY_REGION_KERNEL: return "KERNEL";
		case MEMORY_REGION_RESERVED: return "RESERVED";
		case MEMORY_REGION_INVALID:
		default: 
			return "INVALID";
	}
}

void pmm_print_memory_map(uint64_t base_addr, size_t size) {
	int align = Align(base_addr);

	ks.dbg("Memory map from %x", base_addr);
 	ks._put("{%x - %x}\t", base_addr, base_addr+64*PHYSMEM_BLOCK_SIZE);
	for (int block = 0; block < size; block++) {
		if (block % 64 == 0 && block != 0) 
			ks._put("\n{%x - %x}\t%b", 
					base_addr+(block*PHYSMEM_BLOCK_SIZE), 
					base_addr+(block*PHYSMEM_BLOCK_SIZE)+(64*PHYSMEM_BLOCK_SIZE), 
					pmm_map_get(align+block)
					);
		else if (block % 32 == 0 && block != 0) 
			ks._put("\t%b", pmm_map_get(align+block));
		else
			ks._put("%b", pmm_map_get(align+block));
	}

	ks._put("\n");
}

// *Throw a fatal exception. This should be raised when out of physical memory
void inline pmm_fatal() {
	ks.fatal(FatalError(OUT_OF_MEMORY, "Out of physical memory!"));
}

// === PUBLIC FUNCTIONS =========================

// *Initialize the physical memory manager
// @param entries a valid array of memory_physical_region entries
// @param size the size of the array of memory_physical_region
void init_pmm(MemoryPhysicalRegion* entries, uint32_t size) {
    ks.log("Initializing PMM...");
    
    pmm.regions = entries;
	pmm.regions_count = size;
    pmm.total_memory = entries[size-1].limit - entries[0].base - pmm_get_region_by_type(MEMORY_REGION_FRAMEBUFFER).size;
    pmm.total_blocks = pmm.total_memory / PHYSMEM_BLOCK_SIZE;
    pmm.usable_memory = 0;
	pmm._map = 0;
	pmm._map_size = pmm.total_blocks / PHYSMEM_MAP_BLOCKS_PER_UNIT;

    for (int i = 0; i < size; i++) {
        MemoryPhysicalRegion entry = entries[i];

		ks.dbg("Region #%i: base: %x length: %u type: %c", i, entry.base, entry.size, pmm_region_type_string(entry.type));
        
		// find the first usable region and set it as the base address of the memory bitmap
        if (entry.type == MEMORY_REGION_USABLE && pmm._map == 0 && entry.size >= pmm._map_size && entry.base >= PHYSMEM_2MEGS) {
			pmm._map = (uint32_t*)get_mem_address(entry.base);
			memory_set((uint8_t*)pmm._map, 0, pmm._map_size);
		}
        
		// the usable_memory is the sum of all the usable memory regions
	    if (entry.type == MEMORY_REGION_USABLE) pmm.usable_memory += entry.size;
    }

	pmm.usable_blocks = pmm.usable_memory / PHYSMEM_BLOCK_SIZE;
	// if ((uint64_t)pmm._map != 0) pmm_mark_region_used(0, pmm._map_size - (uint64_t)pmm._map); 

	// mark the memory bitmap itself as used
	pmm_mark_region_used(get_rmem_address((uint64_t)pmm._map), pmm._map_size);
	pmm_map_set(0);	//first block is always set. This insures allocs cant be 0

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
	if (pmm.used_blocks >= pmm.usable_blocks) pmm_fatal();
	
	uint32_t block = pmm_map_first_free();
	if (block == -1) pmm_fatal();
	
	pmm_map_set(block);
	pmm_update_blocks(+1);

	return (uintptr_t)(block*PHYSMEM_BLOCK_SIZE);
}

// *Allocate a physical memory block, clear all the bits and return the physical address of the assigned region
// @return the physical address of the assigned block
uintptr_t pmm_alloc_zero() {
	uintptr_t frame = pmm_alloc();
	memory_set((uint8_t*)frame, 0, PHYSMEM_BLOCK_SIZE);
	return frame;
}

// *Free a physical memory block
// @param addr the address of the physical memory block to free
void pmm_free(uintptr_t addr) {
	uint32_t p = (uint32_t)addr;
	uint32_t block = Align(p);

	pmm_map_unset(block);
	pmm_update_blocks(-1);
}

// *Allocate a series physical memory blocks and return the physical address of the assigned region
// @param size the number of physical memory blocks to allocate
// @return the physical address of the assigned block
uintptr_t pmm_alloc_series(size_t size) {
	if (pmm.used_blocks + size >= pmm.usable_blocks) pmm_fatal();

	uint32_t block = pmm_map_first_free_series(size);
	if (block == -1) pmm_fatal();

	for (uint32_t i=0; i<size; i++) pmm_map_set(block+i);
	pmm_update_blocks(size);

	return (uintptr_t)(block*PHYSMEM_BLOCK_SIZE);
}

// *Free a series of physical memory block
// @param size the number of physical memory blocks to free
// @param addr the address of the physical memory block to free
void pmm_free_series(uintptr_t addr, size_t size) {
	uint32_t p = (uint32_t)addr;
	uint32_t block = Align(p);

	for (uint32_t i=0; i<size; i++) pmm_map_unset(block+i);
	pmm_update_blocks(-size);
}

// *Get the base address of a memory region given the type. Only return the first region found
// @param type the type of memory region to get
// @return the base address of the memory region, 0 if not found 
MemoryPhysicalRegion pmm_get_region_by_type(memory_physical_region_type type) {
	for (uint32_t i = 0; i < pmm.regions_count; i++) {
		if (pmm.regions[i].type != type) continue;
		return (MemoryPhysicalRegion)pmm.regions[i];
	}

	return pmm.regions[0];
}
