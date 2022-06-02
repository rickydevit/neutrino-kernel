#pragma once
#include <stdint.h>
#include <neutrino/macros.h>

#define CONFIG_ADDR_PORT	0xcf8
#define CONFIG_DATA_PORT	0xcfc

#define ADDR_ENABLE		((uint32_t)1 << 31)
#define AddrBusID(x)	(((uint32_t)(x) & 0xff) << 16)
#define AddrDevID(x)	(((uint32_t)(x) & 0x1f) << 11)
#define AddrFunID(x)	(((uint32_t)(x) & 3) << 8)

#define INVALID_VENDOR  0xffff

typedef struct __pci_header_type {
    uint8_t multifunction : 1;
    uint8_t type : 7;
} packed PCIHeaderType;

typedef struct __pci_config_data_common {
    uint16_t vendor, device;
	uint16_t cmd, status;
	uint8_t rev, iface, subclass, class_code;
	uint8_t cacheline_size, latency_timer;
    PCIHeaderType header_type;
    uint8_t bist;
} packed PCIConfigDataCommon;

typedef struct __pci_config_data_general_dev {
	PCIConfigDataCommon common;
	uint32_t base_addr[6];
	uint32_t cardbus_cis;
	uint16_t subsys_vendor;
	uint16_t subsys;
	uint32_t rom_addr;
    uint8_t capabilities_ptr;
	uint32_t reserved1 : 24, reserved2;
	uint8_t intr_line, intr_pin;
	uint8_t min_grant, max_latency;
} packed PCIConfigData_GeneralDevice;

typedef struct __pci_config_data_pci_bridge {
	PCIConfigDataCommon common;
	uint32_t base_addr[2];
	uint8_t primary_bus_num, secondary_bus_num;
    uint8_t subord_bus_num, secondary_latency_timer;
    uint8_t io_base, io_limit;
    uint16_t secondary_status;
    uint16_t memory_base, memory_limit;
    uint16_t prefetch_memory_base, prefetch_memory_limit;
    uint32_t prefetch_base_up32;
    uint32_t prefetch_limit_up32;
    uint16_t io_base_up16, io_limit_up16;
    uint8_t capabilities_ptr;
    uint32_t reserved1 : 24;
    uint32_t rom_addr;
    uint8_t intr_line, intr_pin;
    uint16_t bridge_control;
} packed PCIConfigData_PCIBridge;

typedef struct __pci_config_data_cardbus_bridge {
    PCIConfigDataCommon common;
    uint32_t cardbus_base_addr;
    uint8_t capabilities_list_offset, reserved1;
    uint16_t secondary_status;
    uint8_t pci_bus_num, cardbus_bus_num, subord_bus_num, cardbus_latency_timer;
    struct __memory {
        uint32_t memory_base_addr;
        uint32_t memory_limit;
    } memory_address[2];
    struct __io {
        uint32_t io_base_addr;
        uint32_t io_limit;
    } io_address[2];
    uint8_t intr_line, intr_pin;
    uint16_t bridge_control;
    uint16_t subsys_device, subsys_vendor;
    uint32_t legacy_mode_addr;
} packed PCIConfigData_CardBusBridge;

typedef enum __cmd_register_option {
    IO_SPACE = 0b1,
    MEMORY_SPACE = 0b10,
    BUS_MASTER = 0b100,
    SPECIAL_CYCLES = 0b1000,
    MEM_INVALIDATE_ENABLE = 0b10000,
    PALETTE_SNOOP = 0b100000,
    PARITY_ERROR_RESPONSE = 0b1000000,
    SERR_ENABLE = 0b100000000,
    FAST_B2B_ENABLE = 0b1000000000,
    INTERRUPT_DISABLE = 0b10000000000
} CmdRegisterOption;

typedef enum __status_register_option {
    INTERRUPT_STATUS = 0b1000,
    CAPABILITIES_LIST = 0b10000,
    DOUBLEFREQ_CAPABLE = 0b100000,
    FAST_B2B_CAPABLE = 0b10000000,
    MASTER_PARITY_ERROR = 0b100000000,
    DEVSEL_TIMING_MASK = 0b11000000000,
    SIGN_TARGET_ABORT = 0b100000000000,
    RECV_TARGET_ABORT = 0b1000000000000,
    RECV_MASTER_ABORT = 0b10000000000000,
    SIGN_SYSTEM_ERROR = 0b100000000000000,
    DETC_PARITY_ERROR = 0b1000000000000000
} StatusRegisterOption;

void init_pci();
