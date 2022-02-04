#pragma once
#include <stdint.h>
#include <neutrino/macros.h>

#define RSDP_LOW    0x80000
#define RSDP_HIGH   0x100000

struct RSDP_descriptor {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} packed;

struct RSDP_descriptor2 {
    struct RSDP_descriptor firstPart;
    
    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} packed;

struct SDT_header {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} packed;

struct RSDT {
    struct SDT_header h;
    uint32_t PointerToOtherSDT[];
} packed;

struct XSDT {
    struct SDT_header h;
    uint64_t PointerToOtherSDT[];
} packed; 

struct MADT_apic_header {
    uint8_t type;
    uint8_t length;
};

struct MADT {
    struct SDT_header h;
    uint32_t lapic_address;
    uint32_t flags;
    struct MADT_apic_header interrupt_devices_start;
} packed; 

struct __MADT_apic_IOAPIC {
    struct MADT_apic_header h;
    uint8_t apic_id;
    uint8_t reserved;
    uint32_t apic_addr;
    uint32_t gsib;          // global system interrupt base
};

struct __MADT_apic_IOAPIC_ISO {
    struct MADT_apic_header h;
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t gsi;           // global system interrupt
    uint16_t flags;
};

struct __MADT_apic_IOAPIC_NMI {
    struct MADT_apic_header h;
    uint8_t nmi_source;
    uint8_t reserved;
    uint16_t flags;
    uint32_t gsi;           // global system interrupt
};

typedef struct __MADT_apic_IOAPIC MadtApicIOApic;
typedef struct __MADT_apic_IOAPIC_ISO MadtApicIOApicISO;
typedef struct __MADT_apic_IOAPIC_NMI MadtApicIOApicNMI;

struct acpi {
    uint8_t version;
    union {
        struct RSDT *rsdt;
        struct XSDT *xsdt;
    };
};

struct acpi acpi;

void init_acpi();
void *find_sdt_entry(const char* entry_sign);
