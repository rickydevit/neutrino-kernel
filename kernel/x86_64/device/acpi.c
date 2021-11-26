#include "acpi.h"
#include "libs/_null.h"
#include "libs/libc/stdint.h"
#include "libs/libc/stdbool.h"
#include "libs/libc/size_t.h"
#include "kernel/common/kservice.h"
#include "kernel/common/memory/memory.h"
#include "../memory/mem_virt.h"

// === PRIVATE FUNCTIONS ========================

// *Find the RSDP descriptor and return a pointer to it
// @return a pointer to the RSDP descriptor
void* find_rsdp() {
    for (uint32_t i = RSDP_LOW; i < RSDP_HIGH; i+=0x10) {
        char* candidate = (char*)i;
        if (strncmp(candidate, "RSD PTR ", 8) == 0) return (void*)candidate;
    }

    return 0;
}

// *Do the checksum calculation, valid for each acpi related table
// @param start the pointer to the start of the table
// @param length the length of the table in byte
// @return true if the table is valid, false otherwise
bool do_checksum(void* start, size_t length) {
    uint8_t checksum = 0;
    for (uint64_t i = 0; i < length; i++) 
        checksum += ((int8_t*)start)[i];
        
    return checksum == 0;
}

// === PUBLIC FUNCTIONS =========================

void init_acpi() {
    struct RSDP_descriptor2* descriptor = (struct RSDP_descriptor2*) find_rsdp();
    if (descriptor == 0) {
        ks.err("Cannot find RSDP descriptor. ACPI functionalities may not be available");
        return;
    }

    acpi.version = descriptor->firstPart.Revision;
    ks.log("RSDP found at %x. Version is %i", descriptor, descriptor->firstPart.Revision);

    // Validate the RSDP and load RSDT or XSDT based on acpi version
    if (acpi.version == 0) {
        if (!do_checksum((uint8_t*)descriptor, sizeof(struct RSDP_descriptor))) 
            ks.warn("Error while validating the RSDP. It may be corrupted");

        acpi.rsdt = (struct RSDT*) descriptor->firstPart.RsdtAddress;
    } else {
        if (!do_checksum((uint8_t*)descriptor, sizeof(struct RSDP_descriptor)) || 
            !do_checksum(((uint8_t*)descriptor + sizeof(struct RSDP_descriptor)), sizeof(struct RSDP_descriptor2) - sizeof(struct RSDP_descriptor))) 
            ks.warn("Error while validating the RSDP. It may be corrupted");

        acpi.xsdt = (struct XSDT*) descriptor->XsdtAddress;
    }
}

// *Find a SDT table for the provided signature and return its address
// @param entry_sign the signature of the entry to find
// @return the address of the table, or NULL if not found
void *find_sdt_entry(const char* entry_sign) {
    uint64_t sdt_size = 0, entries = 0;

    if (acpi.version == 0) {
        sdt_size = acpi.rsdt->h.Length;
        entries = (acpi.rsdt->h.Length - sizeof(acpi.rsdt->h)) / 4;

        if (!do_checksum(acpi.rsdt, sdt_size)) 
            ks.warn("Error while validating the RSDT. It may be corrupted");
        
        for (int i = 0; i < entries; i++) {
            struct SDT_header *h = (struct SDT_header *) acpi.rsdt->PointerToOtherSDT[i];
            if (!strncmp(h->Signature, entry_sign, 4))
                return (void *) h;
        }
    } else {
        sdt_size = acpi.xsdt->h.Length;
        entries = (acpi.xsdt->h.Length - sizeof(acpi.xsdt->h)) / 8;

        if (!do_checksum(acpi.xsdt, sdt_size))
            ks.warn("Error while validating the XSDT. It may be corrupted");
        
        for (int i = 0; i < entries; i++) {
            struct SDT_header *h = (struct SDT_header *) acpi.xsdt->PointerToOtherSDT[i];
            if (!strncmp(h->Signature, entry_sign, 4))
                return (void *) h;
        }
    }

    ks.warn("Could not find SDT entry with signature '%c'", entry_sign);

    return (void *)NULL;
}
