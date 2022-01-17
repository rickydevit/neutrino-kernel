#pragma once
#include "stdbool.h"
#include "stdint.h"
#include "../acpi.h"

enum HPET_register_offsets {
    hpet_general_capabilities   = 0x000,
    hpet_general_configuration  = 0x010,
    hpet_general_int_status     = 0x020,
    hpet_main_counter_value     = 0x0f0,
};

#define HPET_ENABLE_GENERAL         0b1
#define HPET_DISABLE_GENERAL        0b0
#define HPET_ENABLE_LEGACY_REPLACE  0b10
#define HPET_DISABLE_LEGACY_REPLACE 0b00

struct address_structure {
    uint8_t address_space_id;    // 0 - system memory, 1 - system I/O
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
} __attribute__((packed));

struct HPET {
    struct SDT_header header;
    uint8_t hardware_rev_id;
    uint8_t comparator_count : 5;
    uint8_t counter_size : 1;
    uint8_t reserved : 1;
    uint8_t legacy_replacement : 1;
    uint16_t pci_vendor_id;
    struct address_structure address;
    uint8_t hpet_number;
    uint16_t minimum_tick;
    uint8_t page_protection;
} __attribute__((packed));

struct hpet {
    struct HPET* table_pointer;
    uint64_t base;
    uint64_t clock_period;
};

struct hpet hpet;

bool has_hpet();
void init_hpet();
void hpet_sleep(uint64_t ms);
