#pragma once
#include <stdint.h>
#include <neutrino/macros.h>

//? 0: null descriptor
//? 1: 32 bit code descriptor
//? 2: 32 bit data descriptor
//? 3: 64 bit code descriptor
//? 4: 64 bit data descriptor
#define GDT_ENTRIES 5

#define GDT_PRESENT     0b10010000	        // Present bit. This must be 1 for all valid selectors.
#define GDT_USER        0b01100000		    // Privilege, 2 bits. Contains the ring level, 0 = highest (kernel), 3 = lowest (user applications).
#define GDT_EXECUTABLE  0b00001000          // Executable bit. If 1 code in this segment can be executed, ie. a code selector. If 0 it is a data selector.
#define GDT_READWRITE   0b00000010          // Readable bit for code selectors. Writable bit for data selectors
#define GDT_ACCESSED    0b00000001
#define GDT_TSS_PRESENT 0b10000000
#define GDT_TSS			0b00001001

#define GDT_FLAGS_64BIT 		0b1000
#define GDT_FLAGS_64BIT_CODE 	0b1010
#define GDT_FLAGS_32BIT 		0b1100
#define GDT_FLAGS_TSS			0b0000 		

struct GDT_pointer {
	uint16_t size;
	uint64_t offset;
} packed;

struct GDT_entry {
	uint16_t limit0_15;
	uint16_t base0_15;
	uint8_t base16_23;
	uint8_t access;
	uint8_t limit16_19 : 4;
	uint8_t flags : 4;
	uint8_t base24_31;
};

struct TSS_entry {
	struct GDT_entry low;	
	struct high_t {
		uint32_t base32_63;
		uint32_t reserved;
	} high;
};

struct cpu_GDT {
	struct GDT_entry GDT[GDT_ENTRIES];
	struct TSS_entry TSS;
};

struct __tss {
	uint32_t reserved0 __attribute__((aligned(16)));
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	
	uint64_t reserved1;
	uint64_t ist1, ist2, ist3, ist4, ist5, ist6, ist7;

	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t iopb_offset;
} packed;

typedef struct __tss Tss;

#include "smp.h"

struct GDT_entry inline gdt_entry_create(uint64_t base, uint64_t limit, uint64_t access, uint64_t flags) {
	struct GDT_entry entry;

	entry.limit0_15 = (uint16_t)(limit & 0xffff);
	entry.limit16_19 = (uint16_t)((limit >> 16) & 0x0f);
	entry.base0_15 = (uint16_t)(base & 0xffff);
	entry.base16_23 = (uint8_t)((base >> 16) & 0xff);
	entry.base24_31 = (uint8_t)((base >> 24) & 0xff);
	entry.access = access;
	entry.flags = flags;

	return entry;
}

struct TSS_entry inline tss_entry_create(uint64_t base, uint64_t limit, uint64_t access, uint64_t flags) {
	struct TSS_entry entry;

	entry.low = gdt_entry_create(base, limit, access, flags);
	entry.high.base32_63 = ((base) >> 32) & 0xffffffff;
	entry.high.reserved = 0;

	return entry;
}

void init_gdt();
void init_gdt_on_ap(uint32_t cpu_id);
void init_tss(struct cpu* cpu);
