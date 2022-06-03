#pragma once
#include <neutrino/macros.h>

#define OTHER 0x80

// --- Class codes ---

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

// --- Subclass codes ---

#define FOREACH_SUBCLASS_UNCLASSIFIED(c) \
    c(NON_VGA_COMPATIBLE = 0x0) \
    c(VGA_COMPATIBLE = 0x1) \

#define FOREACH_SUBCLASS_MASS_STORAGE(c) \
    c(SCSI_BUS_CONTROLLER = 0x0) \
    c(IDE_CONTROLLER = 0x1) \
    c(FLOPPY_DISK_CONTROLLER = 0x2) \
    c(IPI_BUS_CONTROLLER = 0x3) \
    c(RAID_CONTROLLER = 0x4) \
    c(ATA_CONTROLLER = 0x5) \
    c(SERIAL_ATA_CONTROLLER = 0x6) \
    c(SERIAL_SCSI_CONTROLLER = 0x7) \
    c(NON_VOLATILE_MEMORY_CONTROLLER = 0x8) \

#define FOREACH_SUBCLASS_NETWORK(c) \
    c(ETHERNET_CONTROLLER = 0x0) \
    c(TOKEN_RING_CONTROLLER = 0x1) \
    c(FDDI_CONTROLLER = 0x2) \
    c(ATM_CONTROLLER = 0x3) \
    c(ISDN_CONTROLLER = 0x4) \
    c(WORLDFIP_CONTROLLER = 0x5) \
    c(PICMG_CONTROLLER = 0x6) \
    c(INFINIBAND_CONTROLLER = 0x7) \
    c(FABRIC_CONTROLLER = 0x8) \

#define FOREACH_SUBCLASS_DISPLAY(c) \
    c(VGA_COMPATIBLE_CONTROLLER = 0x0) \
    c(XGA_CONTROLLER = 0x1) \
    c(NON_VGA_3D_CONTROLLER = 0x2) \

#define FOREACH_SUBCLASS_BRIDGE(c) \
    c(HOST_BRIDGE = 0x0) \
    c(ISA_BRIDGE = 0x1) \
    c(EISA_BRIDGE = 0x2) \
    c(MCA_BRIDGE = 0x3) \
    c(PCI2PCI_BRIDGE = 0x4) \
    c(PCMCIA_BRIDGE = 0x5) \
    c(NUBUS_BRIDGE = 0x6) \
    c(CARDBUS_BRIDGE = 0x7) \
    c(RACEWAY_BRIDGE = 0x8) \
    c(PCI2PCI_BRIDGE2 = 0x9) \
    c(INFINIBAND2PCI_BRIDGE = 0xA) \

#define FOREACH_SUBCLASS_SERIAL_BUS(c) \
    c(FIREWIRE_CONTROLLER = 0x0) \
    c(ACCESS_BUS_CONTROLLER = 0x1) \
    c(SSA = 0x2) \
    c(USB_CONTROLLER = 0x3) \
    c(FIBRE_CHANNEL = 0x4) \
    c(SMBUS_CONTROLLER = 0x5) \
    c(SERIAL_INFINIBAND_CONTROLLER = 0x6) \
    c(IPMI_INTERFACE = 0x7) \
    c(SERCOS_INTERFACE = 0x8) \
    c(CANBUS_CONTROLLER = 0x9) \

typedef enum __unclassified_subclass {
    FOREACH_SUBCLASS_UNCLASSIFIED(GenerateEnum)
} UnclassifiedSubclass;

typedef enum __mass_storage_subclass {
    FOREACH_SUBCLASS_MASS_STORAGE(GenerateEnum)
} MassStorageSubclass;

typedef enum __network_subclass {
    FOREACH_SUBCLASS_NETWORK(GenerateEnum)
} NetworkSubclass;

typedef enum __display_subclass {
    FOREACH_SUBCLASS_DISPLAY(GenerateEnum)
} DisplaySubclass;

typedef enum __bridge_subclass {
    FOREACH_SUBCLASS_BRIDGE(GenerateEnum)
} BridgeSubclass;

typedef enum __serial_bus_subclass {
    FOREACH_SUBCLASS_SERIAL_BUS(GenerateEnum)
} SerialBusSubclass;

static const char* subclass_unclassified_str[] = {
    FOREACH_SUBCLASS_UNCLASSIFIED(GenerateString)
};

static const char* subclass_mass_storage_str[] = {
    FOREACH_SUBCLASS_MASS_STORAGE(GenerateString)
};

static const char* subclass_network_str[] = {
    FOREACH_SUBCLASS_NETWORK(GenerateString)
};

static const char* subclass_display_str[] = {
    FOREACH_SUBCLASS_DISPLAY(GenerateString)
};

const char* subclass_bridge_str[] = {
    FOREACH_SUBCLASS_BRIDGE(GenerateString)
};

const char* subclass_serial_bus_str[] = {
    FOREACH_SUBCLASS_SERIAL_BUS(GenerateString)
};

static inline const char* pci_get_subclass_string(ClassCode class_code, uint32_t subclass) {
    if (subclass == OTHER) return "UNKNOWN DEVICE = 0x80";

    switch (class_code) {
        case UNCLASSIFIED: return subclass_unclassified_str[subclass];
        case MASS_STORAGE_CONTROLLER: return subclass_mass_storage_str[subclass];
        case NETWORK_CONTROLLER: return subclass_network_str[subclass];
        case DISPLAY_CONTROLLER: return subclass_display_str[subclass];
        case BRIDGE: return subclass_bridge_str[subclass];
        case SERIAL_BUS_CONTROLLER: return subclass_serial_bus_str[subclass];
        default: return "INVALID SUBCLASS";
    };
}
