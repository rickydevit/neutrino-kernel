#include "pci.h"
#include "class_codes.h"
#include <size_t.h>
#include <liballoc.h>
#include "kernel/common/memory/memory.h"
#include "kernel/common/device/port.h"
#include "kernel/common/kservice.h"

// === PRIVATE FUNCTIONS ========================

void pci_check_fun(uint8_t bus, uint8_t dev, uint8_t fun);
void pci_check_dev(uint8_t bus, uint8_t dev);
void pci_check_bus(uint8_t bus);

uint16_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, size_t offset) {
    uint32_t address = ADDR_ENABLE | AddrBusID(bus) | AddrDevID(slot) | AddrFunID(func) | (offset & 0xfc);
    port_dword_out(CONFIG_ADDR_PORT, address);

    //? (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    uint16_t tmp = (uint16_t)(port_dword_in(CONFIG_DATA_PORT) >> ((offset & 2) * 8) & 0xffff);
    return tmp;
}

uint16_t pci_get_vendor(uint8_t bus, uint8_t dev, uint8_t fun) {
    return pci_read_config(bus, dev, fun, GetMemberOffset(PCIConfigDataCommon, vendor));
}

PCIHeaderType pci_get_header(uint8_t bus, uint8_t dev, uint8_t fun) {
    uint8_t res = (uint8_t)pci_read_config(bus, dev, fun, GetMemberOffset(PCIConfigDataCommon, header_type));
    return (PCIHeaderType) {
        .multifunction = (res & 0x80) >> 7,
        .type = res & 0x7f
    };
}

ClassCode pci_get_class(uint8_t bus, uint8_t dev, uint8_t fun) {
    return (ClassCode)(pci_read_config(bus, dev, fun, GetMemberOffset(PCIConfigDataCommon, class_code)) >> 8);
}

uint16_t pci_get_subclass(uint8_t bus, uint8_t dev, uint8_t fun) {
    return (pci_read_config(bus, dev, fun, GetMemberOffset(PCIConfigDataCommon, subclass)) & 0xff);
}

PCIConfigDataCommon* pci_read_full_config(uint8_t bus, uint8_t dev, uint8_t fun) {
    PCIHeaderType hdr = pci_get_header(bus, dev, fun);
    size_t hdr_size = ((hdr.type == 0x0) ? sizeof(PCIConfigGeneralDevice) : 
                                       (hdr.type == 0x1) ? sizeof(PCIConfigPCIBridge) : 
                                       sizeof(PCIConfigCardBusBridge));
    
    PCIConfigDataCommon* res = (PCIConfigDataCommon*)kmalloc(hdr_size);
    for (size_t offset = 0; offset < hdr_size/2; offset++)
        *((uint16_t*)res+offset) = pci_read_config(bus, dev, fun, offset*2);
    
    return res;
}

void pci_print_device(uint8_t bus, uint8_t dev, uint8_t fun, PCIConfigDataCommon* common) {
    ks.dbg("PCI - (%u:%u.%u) device %x:%x class: %c - subclass: %c", bus, dev, fun, pci_get_vendor(bus, dev, fun), 
        common->device, pci_get_class_string(common->class_code), pci_get_subclass_string(common->class_code, common->subclass));

    if (common->header_type.type == 0x0) {
        PCIConfigGeneralDevice* h = (PCIConfigGeneralDevice*)common;

        size_t i = 0;
        while (i<6) {
            if (h->base_addr[i].bar != 0) {
                uintptr_t addr = GetBARAddr(h->base_addr, i);
                ks.dbg("      BAR%u: %c space at %x", i, (h->base_addr[i].bar_type == BAR_MEMORY_SPACE) ? "memory" : "io", addr); 
            }
            i += (h->base_addr[i].bar_type == BAR_MEMORY_SPACE && h->base_addr[i].memory_bar_type == MEMORY_BAR_64BIT) ? 2 : 1;
        }
    }
}

void pci_check_fun(uint8_t bus, uint8_t dev, uint8_t fun) {
    ClassCode base_class = pci_get_class(bus, dev, fun);
    uint16_t sub_class = pci_get_subclass(bus, dev, fun);

    if (base_class == BRIDGE && sub_class == PCI2PCI_BRIDGE) {
        uint16_t secondary_bus = pci_read_config(bus, dev, fun, GetMemberOffset(PCIConfigPCIBridge, secondary_bus_num));
        pci_check_bus(secondary_bus);
   }
}

void pci_check_dev(uint8_t bus, uint8_t dev) {
    if (pci_get_vendor(bus, dev, 0) == INVALID_VENDOR) return;
    pci_check_fun(bus, dev, 0);

    pci_print_device(bus, dev, 0, pci_read_full_config(bus, dev, 0));

    // device is multifunction, check other functions
    if (pci_get_header(bus, dev, 0).multifunction) 
        for (size_t fun = 1; fun < 8; fun++) {
            if (pci_get_vendor(bus, dev, fun) != INVALID_VENDOR) {
                pci_check_fun(bus, dev, fun);
                pci_print_device(bus, dev, fun, pci_read_full_config(bus, dev, fun));
            }
        }
}

void pci_check_bus(uint8_t bus) {
    for (size_t dev = 0; dev < 32; dev++)
        pci_check_dev(bus, dev);
}

void pci_check_all_buses() {
    PCIHeaderType header = pci_get_header(0, 0, 0);
    if (!(header.multifunction)) {
        pci_check_bus(0);
    } else {
        for (size_t fun = 0; fun < 8; fun++) {
            if (pci_get_vendor(0, 0, fun) != INVALID_VENDOR) break;
            pci_check_bus(fun);
        }
    }
}

// ===  PUBLIC FUNCTIONS ========================

void init_pci() {
    pci_check_all_buses();
    ks.log("PCI devices enumeration complete");
}
