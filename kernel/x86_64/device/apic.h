#pragma once
#include "stdint.h"
#include "acpi.h"

#define LAPIC_ENABLE (1 << 10)
#define LAPIC_TIMER_MASKED (1 << 16)

enum apic_register {
    lapic_id =  0x20,
    eoi =       0xb0,   // EOI register
    sivr =      0xf0,  // spurious interrupt vector register
    icr1 =      0x300, // interrupt command register 0-31 
    icr2 =      0x310, // interrupt command register 32-63
    lvt_timer = 0x320,
    lint0 =     0x350, // interrupt command register lint0
    lint1 =     0x360, // interrupt command register lint1

    timer_div =             0x3E0,
    timer_init_counter =    0x380,
    timer_current =         0x390
};

enum apic_timer_mode {
    apic_mode_one_shot,
    apic_mode_periodic,
    apic_mode_tsc_deadline
};

enum apic_timer_division {
    apic_timer_divide_by_2 = 0,
    apic_timer_divide_by_4 = 1,
    apic_timer_divide_by_8 = 2,
    apic_timer_divide_by_16 = 3,
    apic_timer_divide_by_32 = 4,
    apic_timer_divide_by_64 = 5,
    apic_timer_divide_by_128 = 6,
    apic_timer_divide_by_1 = 7
};

enum ioapic_register {
    version_reg = 0x1
};

enum ioapic_flags {
    active_high_low = 2,
    edge_level = 8
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

struct io_apic_version_table {
    uint8_t version;
    uint8_t reserved;
    uint8_t maximum_redirection;
    uint8_t reserved2;
};

struct apic_t {
    uint64_t apic_addr;

    uint16_t ioapics_count;
    MadtApicIOApic* ioapics[64];

    uint16_t ioapics_iso_count;
    MadtApicIOApicISO* ioapics_iso[64];
};

struct apic_t apic;

void init_apic();
void enable_apic();
void apic_eoi();
void apic_write(uint32_t reg, uint32_t value);
uint32_t apic_read(enum apic_register reg);
void map_apic_into_space();
void apic_redirect_irq(uint32_t cpu, uint8_t irq, uint32_t status);
void apic_set_legacy_irq_redirect();
