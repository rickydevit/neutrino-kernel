#include "apic.h"
#include "arch.h"
#include "smp.h"
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

// *Write a value to the specified IOAPIC register
// @param base the base address of the IOAPIC
// @param reg the APIC register to write to
// @param value the value to write
void apic_io_write(uint64_t base, uint32_t reg, uint32_t value) {
    *((volatile uint32_t*)(base)) = reg;
    *((volatile uint32_t*)(base+16)) = value;
}

// *Read a value from the specified IOAPIC register
// @param base the base address of the IOAPIC
// @param reg the APIC register to write to
// @return  value the value to write
uint32_t apic_io_read(uint64_t base, uint32_t reg) {
    *((volatile uint32_t*)(base)) = reg;
    return *((volatile uint32_t*)(base+16));
}

// *Setup the I/O APIC 
void apic_setup_IOAPIC() {
    struct MADT* madt = (struct MADT*) find_sdt_entry("APIC");
    struct MADT_apic_header* entry_hdr = &(madt->interrupt_devices_start);
    apic.ioapics_count = 0;

    while (entry_hdr < ((uint64_t)madt + madt->h.Length)) {
        if (entry_hdr->type == IOAPIC) {
            apic.ioapics[apic.ioapics_count] = (MADT_apic_IOAPIC*) entry_hdr;
            vmm_map_page_in_active_table(apic.ioapics[apic.ioapics_count]->apic_addr, apic.ioapics[apic.ioapics_count]->apic_addr, true, false);
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

uint32_t apic_io_max_redirect(uint32_t apic_id) {
    uint64_t addr = apic.ioapics[apic_id]->apic_addr;
    uint32_t raw_t = apic_io_read(addr, version_reg);

    struct io_apic_version_table* tables = (struct io_apic_version_table*)&raw_t;
    return tables->maximum_redirection;
}

void apic_set_raw_redirect(uint8_t vector, uint32_t target_gsi, uint16_t flags, uint32_t cpu, uint32_t status) {
    uint64_t end = vector;

    int64_t io_apic_target = -1;
    for (uint64_t i = 0; i < apic.ioapics_count; i++) {
        if (apic.ioapics[i]->gsib <= target_gsi && 
            apic.ioapics[i]->gsib + apic_io_max_redirect(i) > target_gsi) {
                io_apic_target = i;
                break;
        }
    }

    if (io_apic_target == -1) {
        ks.err("Error while setting up raw redirect. No ISO table found.");
        return;
    }

    if (flags & active_high_low) end |= (1 << 13);
    if (flags & edge_level) end |= (1 << 15);
    if (!status) end |= (1 << 16);

    end |= get_cpu(cpu)->lapic_id << 56;
    uint32_t io_reg = (target_gsi - apic.ioapics[io_apic_target]->gsib) *2 +16;

    apic_io_write(apic.ioapics[io_apic_target]->apic_addr, io_reg, (uint32_t)end);
    apic_io_write(apic.ioapics[io_apic_target]->apic_addr, io_reg + 1, (uint32_t)(end >> 32));
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

void apic_redirect_irq(uint32_t cpu, uint8_t irq, uint32_t status) {
    ks.dbg("setting redirect for cpu #%u, irq: %u status: %x", cpu, irq, status);

    for (uint64_t i = 0; i < apic.ioapics_iso_count; i++) {
        if (apic.ioapics_iso[i]->irq_source == irq) {
            ks.dbg("matching at %u to source %u, gsi: %x", i, apic.ioapics_iso[i]->irq_source + 0x20, apic.ioapics_iso[i]->gsi);
            apic_set_raw_redirect(apic.ioapics_iso[i]->irq_source + 0x20, apic.ioapics_iso[i]->gsi, apic.ioapics_iso[i]->flags, cpu, status);
            return;
        }
    }

    apic_set_raw_redirect(irq + 0x20, irq, 0, cpu, status);
}

// *Send a EOI to the APIC
void apic_eoi() {
    apic_write(eoi, 0);
}
