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

    // todo ioapic
    // todo iso table
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
