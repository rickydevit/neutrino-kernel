#include "apic.h"
#include "arch.h"
#include "memory/mem_virt.h"
#include "device/acpi.h"
#include "kernel/common/kservice.h"
#include "kernel/common/device/port.h"

// === PRIVATE FUNCTIONS ========================

// *Disable the 8259 PIC
void disable_pic() {
    portByteOut(0xa1, 0xff);
    portByteOut(0x21, 0xff);
}

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

    for (int i = 0; i < 3; i++) 
        vmm_map_page_in_active_table((uint64_t)madt->lapic_address + i * PAGE_SIZE, (uint64_t)apic.apic_addr + i * PAGE_SIZE, true, false);
    
    // enable the APIC
    enable_apic();
    ks.dbg("APIC enabled");

    // disable the PIC
    disable_pic();
    ks.dbg("PIC disabled");

    // todo ioapic
    // todo iso table
}

void enable_apic() {
    write_msr(APIC, (read_msr(APIC) | 0x800) & ~(LAPIC_ENABLE));
    apic_write(sivr, apic_read(sivr) | 0x1ff);
}
