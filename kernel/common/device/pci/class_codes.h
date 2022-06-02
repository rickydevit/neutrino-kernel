#pragma once
#include <neutrino/macros.h>

#define OTHER 0x80

#define FOREACH_CLASS(c)    \
        c(UNCLASSIFIED = 0x0)     \
        c(MASS_STORAGE_CONTROLLER = 0x1) \
        c(NETWORK_CONTROLLER = 0x2) \
        c(DISPLAY_CONTROLLER = 0x3) \
        c(MULTIMEDIA_CONTROLLER = 0x4) \
        c(MEMORY_CONTROLLER = 0x5) \
        c(BRIDGE = 0x6) \
        c(SIMPLE_COMMS_CONTROLLER = 0x7) \
        c(BASE_SYSTEM_PERIPHERAL = 0x8) \
        c(INPUT_DEVICES_CONTROLLER = 0x9) \
        c(DOCKING_STATION = 0xa) \
        c(PROCESSOR = 0xb) \
        c(SERIAL_BUS_CONTROLLER = 0xc) \
        c(WIRELESS_CONTROLLER = 0xd) \
        c(INTELLIGENT_CONTROLLER = 0xe) \
        c(SATELLITE_COMMS_CONTROLLER = 0xf) \
        c(ENCRYPTION_CONTROLLER = 0x10) \
        c(SIGNAL_PROCESSING_CONTROLLER = 0x11) \
        c(PROCESSING_ACCELERATOR = 0x12) \
        c(NON_ESSENTIAL_INSTRUMENTATION = 0x13) \
        c(CO_PROCESSOR = 0x40) \
        c(VENDOR_SPECIFIC = 0xff) \

typedef enum __class_code {
    FOREACH_CLASS(GenerateEnum)
} ClassCode;

static const char* class_code_str[] = {
    FOREACH_CLASS(GenerateString)
};

static inline const char* pci_get_class_string(ClassCode class_code) {
    if (class_code == CO_PROCESSOR) return class_code_str[0x14];
    else if (class_code == VENDOR_SPECIFIC) return class_code_str[0x15];
    else if (class_code <= 0x13) return class_code_str[class_code];
    else return "INVALID CLASS";
}

typedef enum __unclassified_subclass {
    NON_VGA_COMPATIBLE = 0x0,
    VGA_COMPATIBLE = 0x1
} UnclassifiedSubclass;

typedef enum __mass_storage_subclass {
    SCSI_BUS_CONTROLLER = 0x0,
    IDE_CONTROLLER = 0x1,
    FLOPPY_DISK_CONTROLLER = 0x2,
    IPI_BUS_CONTROLLER = 0x3,
    RAID_CONTROLLER = 0x4,
    ATA_CONTROLLER = 0x5,
    SERIAL_ATA_CONTROLLER = 0x6,
    SERIAL_SCSI_CONTROLLER = 0x7,
    NON_VOLATILE_MEMORY_CONTROLLER = 0x8,
} MassStorageSubclass;

typedef enum __network_subclass {
    ETHERNET_CONTROLLER = 0x0,
    TOKEN_RING_CONTROLLER = 0x1,
    FDDI_CONTROLLER = 0x2,
    ATM_CONTROLLER = 0x3,
    ISDN_CONTROLLER = 0x4,
    WORLDFIP_CONTROLLER = 0x5,
    PICMG_CONTROLLER = 0x6,
    INFINIBAND_CONTROLLER = 0x7,
    FABRIC_CONTROLLER = 0x8,
} NetworkSubclass;

typedef enum __display_subclass {
    VGA_COMPATIBLE_CONTROLLER = 0x0,
    XGA_CONTROLLER = 0x1,
    NON_VGA_3D_CONTROLLER = 0x2,
} DisplaySubclass;

typedef enum __bridge_subclass {
    HOST_BRIDGE = 0x0,
    ISA_BRIDGE = 0x1,
    EISA_BRIDGE = 0x2,
    MCA_BRIDGE = 0x3,
    PCI2PCI_BRIDGE = 0x4,
    PCMCIA_BRIDGE = 0x5,
    NUBUS_BRIDGE = 0x6,
    CARDBUS_BRIDGE = 0x7,
    RACEWAY_BRIDGE = 0x8,
    PCI2PCI_BRIDGE2 = 0x9,
    INFINIBAND2PCI_BRIDGE = 0xA,
} BridgeSubclass;

typedef enum __serial_bus_subclass {
    FIREWIRE_CONTROLLER = 0x0,
    ACCESS_BUS_CONTROLLER = 0x1,
    SSA = 0x2,
    USB_CONTROLLER = 0x3,
    FIBRE_CHANNEL = 0x4,
    SMBUS_CONTROLLER = 0x5,
    SERIAL_INFINIBAND_CONTROLLER = 0x6,
    IPMI_INTERFACE = 0x7,
    SERCOS_INTERFACE = 0x8,
    CANBUS_CONTROLLER = 0x9,
} SerialBusSubclass;
