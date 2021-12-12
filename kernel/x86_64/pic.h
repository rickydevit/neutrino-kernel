#pragma once

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

void inline pic_wait() {
    do {
        asm volatile("jmp 1f\n\t"     
                     "1:\n\t"         
                     "    jmp 2f\n\t" 
                     "2:");
    } while (0);
}

void init_pic();
void disable_pic();
