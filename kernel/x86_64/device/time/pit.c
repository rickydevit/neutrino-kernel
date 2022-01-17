#include "pit.h"
#include "kernel/common/kservice.h"
#include "kernel/common/device/port.h"
#include "interrupts.h"
#include "stdint.h"

// === PRIVATE FUNCTIONS ========================

// *Read the current value from the PIT
// @return the current counter value of the PIT
uint16_t pit_read_counter() {
    port_byte_out(command_port, 0);

    uint16_t count = 0;
    count = port_byte_in(channel0_port);
    count |= (port_byte_in(channel0_port) << 8);

    return count;
}

// === PUBLIC FUNCTIONS =========================

// *Initialize the PIT with a given frequency
// @param frequency the frequency to initialize the PIT to
void init_pit(uint64_t target_frequency) {
    ks.fatal(FatalError(NO_PIT, "Programmable Interval Timer is not currently supported"));

    // todo implement PIT as fallback for HPET missing
    // ks.log("Initializing PIT...");
    // uint16_t divisor = (uint16_t)((uint32_t)PIT_FREQ / target_frequency);

    // pit.frequency = target_frequency;
    // pit.ticks = 0;

    // port_byte_out(command_port, _COMMAND(select_channel0, low_high_byte, rate_generator, binary));
    // port_byte_out(channel0_port, (uint8_t)(divisor & 0xff));
    // port_byte_out(channel0_port, (uint8_t)((divisor >> 8) & 0xff));
    
    // apic_redirect_irq(0, 0, 1);
    // ks.log("PIT initialized");
}

// *Wait for the specified number of milliseconds
// @param timeout the number of milliseconds to wait
void pit_wait(uint16_t timeout) {
    ks.fatal(FatalError(NOT_IMPLEMENTED, "pit_wait() not implemented"));
    
    // port_byte_out(command_port, _COMMAND(select_channel0, low_high_byte, int_on_terminal_count, binary));
    // uint16_t to_wait = (uint16_t)((uint32_t)PIT_FREQ / (timeout * 1000));

    // port_byte_out(channel0_port, (uint8_t)(to_wait & 0xff));
    // port_byte_out(channel0_port, (uint8_t)((to_wait >> 8) & 0xff));

    // while ((int64_t) pit_read_counter() != 0);

    // ks.log("time elapsed");
}
