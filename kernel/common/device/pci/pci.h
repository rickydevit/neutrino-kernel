#pragma once
#include "../../memory/memory.h"
#include <stdint.h>
#include <neutrino/macros.h>
#include <_null.h>

#define CONFIG_ADDR_PORT	0xcf8
#define CONFIG_DATA_PORT	0xcfc
#define BAR_LIST_OFFSET     0x10

#define ADDR_ENABLE		((uint32_t)1 << 31)
#define AddrBusID(x)	(((uint32_t)(x) & 0xff) << 16)
#define AddrDevID(x)	(((uint32_t)(x) & 0x1f) << 11)
#define AddrFunID(x)	(((uint32_t)(x) & 3) << 8)

#define GetMemBAR16bitAddr(b)       (uintptr_t)(((BAR)b).bar & 0xfff0)
#define GetMemBAR32bitAddr(b)       (uintptr_t)(((BAR)b).bar & 0xfffffff0)
#define GetMemBAR64bitAddr(b1,b2)   (uintptr_t)(GetMemBAR32bitAddr(b1) + ((uintptr_t)(((BAR)b2).bar & 0xffffffff) << 32))
#define GetIOBARAddr(b)             (uintptr_t)(((BAR)b).bar & 0xfffffffc)

#define GetBARAddr(bar_p,i)         (((BaseAddrReg)bar_p[i]).bar_type == BAR_IO_SPACE) ? GetIOBARAddr(bar_p[i]) : \
                                    (((BaseAddrReg)bar_p[i]).memory_bar_type == MEMORY_BAR_16BIT) ? GetMemBAR16bitAddr(bar_p[i]) : \
                                    (((BaseAddrReg)bar_p[i]).memory_bar_type == MEMORY_BAR_32BIT) ? GetMemBAR32bitAddr(bar_p[i]) : \
                                    (((BaseAddrReg)bar_p[i]).memory_bar_type == MEMORY_BAR_64BIT) ? GetMemBAR64bitAddr(bar_p[i], bar_p[i+1]) : nullptr;

#define INVALID_VENDOR  0xffff

typedef struct __pci_header_type {
    uint8_t type : 7;
    uint8_t multifunction : 1;
} packed PCIHeaderType;

typedef enum __pci_bar_type {
    BAR_MEMORY_SPACE = 0,
    BAR_IO_SPACE = 1,
} BarType;

typedef enum __pci_memory_bar_type {
    MEMORY_BAR_32BIT = 0,
    MEMORY_BAR_16BIT = 1,
    MEMORY_BAR_64BIT = 2,
} MemoryBarType;

typedef union __pci_base_addr_reg {
    BarType bar_type : 1;
    struct {
        MemoryBarType memory_bar_type : 2;
        uint8_t memory_bar_prefetchable : 1;
        uint32_t memory_bar_address : 28;
    };
    struct {
        uint8_t io_bar_reserved : 1;
        uint32_t io_bar_address : 30;
    };
    struct {
        uint32_t bar;
    };
} packed BaseAddrReg;

typedef BaseAddrReg BAR;

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
	BaseAddrReg base_addr[6];
	uint32_t cardbus_cis;
	uint16_t subsys_vendor;
	uint16_t subsys;
	uint32_t rom_addr;
    uint8_t capabilities_ptr;
	uint32_t reserved1 : 24, reserved2;
	uint8_t intr_line, intr_pin;
	uint8_t min_grant, max_latency;
} packed PCIConfigGeneralDevice;

typedef struct __pci_config_data_pci_bridge {
	PCIConfigDataCommon common;
	BaseAddrReg base_addr[2];
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
} packed PCIConfigPCIBridge;

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
} packed PCIConfigCardBusBridge;

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

typedef struct __pci_vendor_device_info {
    uint16_t vendor_id;
    uint16_t device_id;
} PCIVendorDeviceInfo;

typedef struct __pci_location {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
} PCILocation;

typedef struct __pci_bar_info {
    BarType type;
    MemoryRange range;
} PCIBarInfo;

typedef struct _pci_device {
    PCIVendorDeviceInfo dev_info;
    PCILocation location;
    uint8_t class;
    uint8_t subclass;

    size_t bars_count;
    PCIBarInfo* bars;
} PCIDevice;

void init_pci();
PCIDevice* pci_get_device_by_class(uint8_t class, uint8_t subclass);
PCIDevice* pci_get_device_by_vendor(uint16_t vendor_id, uint16_t device_id);
