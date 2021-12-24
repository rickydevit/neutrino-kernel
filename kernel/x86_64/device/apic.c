#include "apic.h"
#include "arch.h"
#include "memory/mem_virt.h"
#include "device/acpi.h"
#include "kernel/common/kservice.h"
#include "kernel/common/device/port.h"

// === PRIVATE FUNCTIONS ========================

// *Write a value to the specified APIC register
// @param reg the APIC register to write to
// @param value the value to write
void apic_write(uint32_t reg, uint32_t value) {
    *((volatile uint32_t *)(((uint64_t)apic.apic_addr) + reg)) = value;
}

// *Read a value from the specified APIC register
// @param reg the APIC register to read from
// @return the value read from the specified APIC register
uint32_t apic_read(uint32_t reg) {
    return *((volatile uint32_t *)((uint64_t)apic.apic_addr + reg));
}

// *Setup the I/O APIC 
void apic_setup_IOAPIC() {
    struct MADT* madt = (struct MADT*) find_sdt_entry("APIC");
    struct MADT_apic_header* entry_hdr = &(madt->interrupt_devices_start);
    apic.ioapics_count = 0;

    while (entry_hdr < ((uint64_t)madt + madt->h.Length)) {
        if (entry_hdr->type == IOAPIC) {
            apic.ioapics[apic.ioapics_count] = (MADT_apic_IOAPIC*) entry_hdr;
            ks.dbg("ioapic #%u addr: %x", apic.ioapics[apic.ioapics_count]->apic_id, apic.ioapics[apic.ioapics_count]->apic_addr);
            apic.ioapics_count++;
        }
        entry_hdr = (struct MADT_apic_header*) ((uint64_t)entry_hdr + entry_hdr->length);
    }
}

// *Setup the I/O APIC Interrupt Source Override
void apic_setup_IOAPIC_ISO() {
    struct MADT* madt = (struct MADT*) find_sdt_entry("APIC");
    struct MADT_apic_header* entry_hdr = &(madt->interrupt_devices_start);
    apic.ioapics_iso_count = 0;

    while (entry_hdr < ((uint64_t)madt + madt->h.Length)) {
        if (entry_hdr->type == IOAPIC_ISO) {
            apic.ioapics_iso[apic.ioapics_iso_count] = (MADT_apic_IOAPIC_ISO*) entry_hdr;
            ks.dbg("ioapic iso bus: %u interrupt: %u", apic.ioapics_iso[apic.ioapics_iso_count]->bus_source, 
                    apic.ioapics_iso[apic.ioapics_iso_count]->irq_source);
            apic.ioapics_iso_count++;
        }
        entry_hdr = (struct MADT_apic_header*) ((uint64_t)entry_hdr + entry_hdr->length);
    }
}

// === PUBLIC FUNCTIONS =========================

void init_apic() {
    ks.log("Initializing APIC...");

    struct MADT *madt = (struct MADT*)find_sdt_entry("APIC");
    if (madt->lapic_address == 0) ks.panic("Cannot find LAPIC address");
    apic.apic_addr = madt->lapic_address;

    map_apic_into_space();      // map the LAPIC address into space
    enable_apic();              // enable the APIC
    ks.dbg("APIC enabled");

    disable_pic();              // disable the PIC
    ks.dbg("PIC disabled");

    ks.dbg("Setting up IOAPICs...");
    apic_setup_IOAPIC();

    ks.dbg("Setting up IOAPIC ISOs...");
    apic_setup_IOAPIC_ISO();
}

// *Map the LAPIC into the active table
void map_apic_into_space() {
    for (int i = 0; i < 3; i++) 
        vmm_map_page_in_active_table((uint64_t)apic.apic_addr + i * PAGE_SIZE, (uint64_t)apic.apic_addr + i * PAGE_SIZE, true, false);
}

// *Enable the APIC by writing to its registers
void enable_apic() {
    write_msr(APIC, (read_msr(APIC) | 0x800) & ~(LAPIC_ENABLE));
    apic_write(sivr, apic_read(sivr) | 0x1ff);
}
