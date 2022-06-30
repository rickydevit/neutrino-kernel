#include "acpi.h"
#include "libs/_null.h"
#include "stdint.h"
#include "stdbool.h"
#include "size_t.h"
#include "string.h"
#include "kernel/common/kservice.h"
#include "kernel/common/memory/memory.h"
#include "../memory/mem_virt.h"
#include "arch.h"
#include <neutrino/macros.h>

// === PRIVATE FUNCTIONS ========================

// *Find the RSDP descriptor and return a pointer to it
// @return a pointer to the RSDP descriptor
void* unoptimized find_rsdp() {
    for (uintptr_t i = get_mem_address(RSDP_LOW); i < get_mem_address(RSDP_HIGH); i+=0x10) {
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

void map_acpi() {
    // map RSDP
    for (uintptr_t addr = RSDP_LOW; addr < RSDP_HIGH; addr += PAGE_SIZE) 
        vmm_map_page(0, addr, get_mem_address(addr), PageKernelWrite);
}

// === PUBLIC FUNCTIONS =========================

void init_acpi() {
    map_acpi();
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

        acpi.rsdt = (struct RSDT*)get_mem_address(descriptor->firstPart.RsdtAddress);
        ks.log("RSDT at %x", descriptor->firstPart.RsdtAddress);
    } else {
        if (!do_checksum((uint8_t*)descriptor, sizeof(struct RSDP_descriptor)) || 
            !do_checksum(((uint8_t*)descriptor + sizeof(struct RSDP_descriptor)), sizeof(struct RSDP_descriptor2) - sizeof(struct RSDP_descriptor))) 
            ks.warn("Error while validating the RSDP. It may be corrupted");

        acpi.xsdt = (struct XSDT*)get_mem_address(descriptor->XsdtAddress);
        ks.log("XSDT at %x", descriptor->XsdtAddress);
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
            struct SDT_header *h = (struct SDT_header *)get_mem_address(acpi.rsdt->PointerToOtherSDT[i]);
            if (!strncmp(h->Signature, entry_sign, 4))
                return (void *) h;
        }
    } else {
        sdt_size = acpi.xsdt->h.Length;
        entries = (acpi.xsdt->h.Length - sizeof(acpi.xsdt->h)) / 8;

        if (!do_checksum(acpi.xsdt, sdt_size))
            ks.warn("Error while validating the XSDT. It may be corrupted");
        
        for (int i = 0; i < entries; i++) {
            struct SDT_header *h = (struct SDT_header *) get_mem_address(acpi.xsdt->PointerToOtherSDT[i]);
            if (!strncmp(h->Signature, entry_sign, 4))
                return (void *) h;
        }
    }

    ks.warn("Could not find SDT entry with signature '%c'", entry_sign);

    return (void *)NULL;
}
