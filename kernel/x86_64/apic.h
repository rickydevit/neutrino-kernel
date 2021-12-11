#pragma once
#include "libs/libc/stdint.h"

#define LAPIC_ENABLE (1 << 10)

enum apic_register {
    lapic_id =  0x20,
    eoi =       0xb0,   // EOI register
    sivr =      0xf0,  // spurious interrupt vector register
    icr1 =      0x300, // interrupt command register
    icr2 =      0x310, // interrupt command register 
    lvt_timer = 0x320,
    lint1 =     0x350, // interrupt command register
    lint2 =     0x360, // interrupt command register 

    timer_div =             0x3E0,
    timer_init_counter =    0x380,
    timer_current =         0x390
};

struct apic_t {
    uint64_t apic_addr;
};

struct apic_t apic;

void init_apic();
void enable_apic();
