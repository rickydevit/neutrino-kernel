#pragma once
#include "libs/libc/stdint.h"

#define PIC1                0x20
#define PIC1_COMMAND        PIC1
#define PIC1_OFFSET         0x20
#define PIC1_DATA           (PIC1 + 1)

#define PIC2                0xa0
#define PIC2_COMMAND        PIC2
#define PIC2_OFFSET         0x28
#define PIC2_DATA           (PIC2 + 1)

#define ICW1_ICW4           0x01
#define ICW1_INIT           0x10
#define PIC_READ_ISR        0x0b
#define PIC_READ_IRR        0x0a
#define PIC_EOI             0x20

void inline pic_wait() {
    do {
        asm volatile("jmp 1f\n\t"     
                     "1:\n\t"         
                     "    jmp 2f\n\t" 
                     "2:");
    } while (0);
}

void init_pic();
void pic_disable();
void pic_eoi();
uint16_t pic_get_irr();
uint16_t pic_get_isr();
