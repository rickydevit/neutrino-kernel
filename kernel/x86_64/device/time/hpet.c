#include "hpet.h"
#include "pit.h"
#include "kernel/common/kservice.h"
#include "../acpi.h"
#include "../../memory/mem_virt.h"
#include "libs/_null.h"
#include "neutrino/macros.h"
#include "stdbool.h"

// === PRIVATE FUNCTIONS ========================

// *Write the value to the given HPET register
// @param reg the register offset
// @param value the value to write
void hpet_write(enum HPET_register_offsets reg, uint64_t value) {
    *((volatile uint64_t*)(hpet.base + reg)) = value;
}

// *Read from the HPET register
// @param reg the register offset
// @return the value read from the register
uint64_t hpet_read(enum HPET_register_offsets reg) {
    return *((volatile uint64_t*)(hpet.base + reg));
}
 
// === PUBLIC FUNCTIONS =========================

// *Check if HPET is available on the system. Call this AFTER calling init_hpet
// @return true if HPET is available on the system, false otherwise
bool has_hpet() { 
    return hpet.table_pointer != NULL;
}

// *Initialize the HPET
void volatile_fun init_hpet() {
    ks.log("Initializing HPET...");
    hpet.table_pointer = (struct HPET*)find_sdt_entry("HPET");

    if (!has_hpet) {
        ks.err("HPET was not found in the system. Cannot initialize it.");
        init_pit(1000);
        return;
    }

    ks.dbg("hpet at %x", hpet.table_pointer->address.address);
    hpet.base = hpet.table_pointer->address.address;

    // map the HPET registers to address space
    ks.dbg("hpet registers at %x", hpet.base);
    hpet.base = vmm_map_mmio((uintptr_t)hpet.base, 1);

    if (hpet.table_pointer->address.address_space_id == 1) {// check for io address space, unsupported 
        ks.err("Unsupported HPET address space");
        init_pit(1000);
        return;
    }

    uint64_t capab = hpet_read(hpet_general_capabilities);

    // save the clock period
    hpet.clock_period = capab >> 32;

    hpet_write(hpet_general_configuration, HPET_DISABLE_GENERAL | HPET_DISABLE_LEGACY_REPLACE); // disable hpet
    hpet_write(hpet_main_counter_value, 0);
    hpet_write(hpet_general_configuration, HPET_ENABLE_GENERAL);

    ks.log("HPET has been initialized. %x", hpet_read(hpet_general_configuration));
}

// *Sleep for a given number of milliseconds. The function will return when the sleep is complete
// @param ms the number of milliseconds to sleep    
void hpet_sleep(uint64_t ms) {
    uint64_t until = hpet_read(hpet_main_counter_value) + (ms * 1000000000000) / hpet.clock_period;
    // now wait until the ms passes
    while (!(hpet_read(hpet_main_counter_value) >= until)) {};
}
