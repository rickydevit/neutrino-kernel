#pragma once
#include "libs/libc/stdint.h"

#define RSDP_LOW    0x80000
#define RSDP_HIGH   0x100000

struct RSDP_descriptor {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} __attribute__ ((packed));

struct RSDP_descriptor2 {
    struct RSDP_descriptor firstPart;
    
    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} __attribute__ ((packed));

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
} __attribute__ ((packed));

struct RSDT {
    struct SDT_header h;
    uint32_t PointerToOtherSDT[];
} __attribute__ ((packed));

struct XSDT {
    struct SDT_header h;
    uint64_t PointerToOtherSDT[];
} __attribute__ ((packed)); 

struct MADT {
    struct SDT_header h;
    uint32_t lapic_address;
    uint32_t flags;
    uint16_t interrupt_devices_start;
} __attribute__ ((packed)); 

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
