#pragma once

#include "libs/libc/stdint.h"

#define INTERRUPT_GATE 0x8e
#define TRAP_GATE 0x8f

#define IDT_SIZE 256

struct IDT_entry {
    uint16_t offset_lowerbits;
    uint16_t selector;
    uint8_t zero;
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
extern int irq0();
extern int irq1();
extern int irq2();
extern int irq3();
extern int irq5();
extern int irq6();
extern int irq7();
extern int irq8();
extern int irq9();
extern int irq10();
extern int irq11();
extern int irq12();
extern int irq13();
extern int irq14();
extern int irq15();

extern int exc0();
extern int exc1();
extern int exc2();
extern int exc3();
extern int exc4();
extern int exc5();
extern int exc6();
extern int exc8();
extern int exc14();