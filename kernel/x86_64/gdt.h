#pragma once
#include "libs/libc/stdint.h"

//? 0: null descriptor
//? 1: 32 bit code descriptor
//? 2: 32 bit data descriptor
//? 3: 64 bit code descriptor
//? 4: 64 bit data descriptor
#define GDT_SIZE 5

#define GDT_PRESENT     0b10010000	        // Present bit. This must be 1 for all valid selectors.
#define GDT_USER        0b01100000		    // Privilege, 2 bits. Contains the ring level, 0 = highest (kernel), 3 = lowest (user applications).
#define GDT_EXECUTABLE  0b00001000          // Executable bit. If 1 code in this segment can be executed, ie. a code selector. If 0 it is a data selector.
#define GDT_READWRITE   0b00000010          // Readable bit for code selectors. Writable bit for data selectors
#define GDT_ACCESSED    0b00000001

#define GDT_FLAGS_64BIT 0b1010
#define GDT_FLAGS_32BIT 0b1100
#define TSS_FLAGS 0

void init_gdt();
// extern void gdt_flush(uint32_t);
// void set_kernel_stack(uint32_t stack);

struct GDT_pointer {
	uint16_t size;
	uint64_t offset;
} __attribute__((packed));

struct GDT_entry {
	uint16_t limit0_15;
	uint16_t base0_15;
	uint8_t base16_23;
	uint8_t acces;
	uint8_t limit16_19 : 4;
	uint8_t flags : 4;
	uint8_t base24_31;
};

#define GDT_ENTRY(__base, __limit, __access, __flags) \
	(struct GDT_entry)                                        \
	{                                                 \
		.limit0_15 = (uint16_t)((__limit)&0xffff),         \
		.base0_15 = (uint16_t)((__base)&0xffff),           \
		.base16_23 = (uint8_t)(((__base) >> 16) & 0xff),   \
		.acces = (__access),                          \
		.limit16_19 = ((__limit) >> 16) & 0x0f,       \
		.flags = (__flags),                           \
		.base24_31 = (uint8_t)(((__base) >> 24) & 0xff),   \
	}
