#pragma once
#include "libs/libc/stdint.h"

#define INTERRUPT_GATE  0x8e
#define TRAP_GATE       0x8f

#define KERNEL_CODE     0x18
#define IDT_SIZE        256

struct IDT_entry {
    uint16_t offset_lowerbits;
    uint16_t selector;
    uint8_t ist : 3;
    uint8_t zero : 5;
    uint8_t type_attr;
    uint16_t offset_middlebits;
    uint32_t offset_higherbits;
    uint32_t zero2;
};

struct IDT_pointer {
    uint16_t size;   
    uint64_t offset; 
} __attribute__((packed));

void init_idt();

extern int load_idt();
extern uint64_t _interrupt_vector[128];
