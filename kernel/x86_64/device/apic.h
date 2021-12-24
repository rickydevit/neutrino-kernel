#pragma once
#include "libs/libc/stdint.h"
#include "acpi.h"

#define LAPIC_ENABLE (1 << 10)

enum apic_register {
    lapic_id =  0x20,
    eoi =       0xb0,   // EOI register
    sivr =      0xf0,  // spurious interrupt vector register
    icr1 =      0x300, // interrupt command register 0-31 
    icr2 =      0x310, // interrupt command register 32-63
    lvt_timer = 0x320,
    lint1 =     0x350, // interrupt command register lint0
    lint2 =     0x360, // interrupt command register lint1

    timer_div =             0x3E0,
    timer_init_counter =    0x380,
    timer_current =         0x390
};

enum apic_type {
    LAPIC = 0,
    IOAPIC = 1,
    IOAPIC_ISO = 2,
    IOAPIC_NMI = 3,
    LAPIC_NMI = 4,
    LAPIC_AO = 5,
    Lx2APIC = 9
};

struct apic_t {
    uint64_t apic_addr;

    uint16_t ioapics_count;
    MADT_apic_IOAPIC* ioapics[64];

    uint16_t ioapics_iso_count;
    MADT_apic_IOAPIC_ISO* ioapics_iso[64];
};

struct apic_t apic;

void init_apic();
void enable_apic();
void map_apic_into_space();
