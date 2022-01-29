#pragma once
#include "stdint.h"

#define PIT_FREQ 1193182
#define _COMMAND(c, a, m, b) ((c << 6) | (a << 4) | (m << 1) | b)

enum pit_port {
    channel0_port = 0x40,    // connected to irq0
    channel1_port = 0x41,    // possibly doesn't exist
    channel2_port = 0x42,    // connected to the pc speaker
    command_port  = 0x43     // write only
};

enum command_select_channel {
    select_channel0 = 0b00,
    select_channel1 = 0b01,
    select_channel2 = 0b10,
    select_read_back = 0b11
};

enum command_access_mode {
    latch_count_value = 0b00,
    low_byte = 0b01,
    high_byte = 0b10,
    low_high_byte = 0b11
};

enum command_operating_mode {
    int_on_terminal_count = 0b000,      
    hw_retriggable_oneshot = 0b001,
    rate_generator = 0b010,
    square_wave_generator = 0b011,
    sw_triggered_strobe = 0b100,
    hw_triggered_strobe = 0b101
};

enum command_bcd_binary {
    binary = 0,
    bcd = 1
};

struct pit {
    uint64_t frequency;
    uint64_t ticks;
};

struct pit pit;

void init_pit(uint64_t target_frequency);
void pit_sleep(uint16_t timeout);
