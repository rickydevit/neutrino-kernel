#include "pci.h"
#include "class_codes.h"
#include <size_t.h>
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
        .multifunction = res & 0x80 >> 7,
        .type = res & 0x7f
    };
}

ClassCode pci_get_class(uint8_t bus, uint8_t dev, uint8_t fun) {
    return (ClassCode)(pci_read_config(bus, dev, fun, GetMemberOffset(PCIConfigDataCommon, class_code)) >> 8);
}

uint16_t pci_get_subclass(uint8_t bus, uint8_t dev, uint8_t fun) {
    return (pci_read_config(bus, dev, fun, GetMemberOffset(PCIConfigDataCommon, subclass)) & 0xff);
}

void pci_print_device(uint8_t bus, uint8_t dev, uint8_t fun) {
        ks.dbg("PCI - (%u:%u.%u) device %x:%x class: %c subclass: %x", bus, dev, fun, pci_get_vendor(bus, dev, fun), 
            pci_read_config(bus, dev, fun, GetMemberOffset(PCIConfigDataCommon, device)), 
            pci_get_class_string(pci_get_class(bus, dev, fun)), pci_get_subclass(bus, dev, fun));
}

void pci_check_fun(uint8_t bus, uint8_t dev, uint8_t fun) {
    ClassCode base_class = pci_get_class(bus, dev, fun);
    uint16_t sub_class = pci_get_subclass(bus, dev, fun);

    if (base_class == BRIDGE && sub_class == PCI2PCI_BRIDGE) {
        uint16_t secondary_bus = pci_read_config(bus, dev, fun, GetMemberOffset(PCIConfigData_PCIBridge, secondary_bus_num));
        pci_check_bus(secondary_bus);
   }
}

void pci_check_dev(uint8_t bus, uint8_t dev) {
    if (pci_get_vendor(bus, dev, 0) == INVALID_VENDOR) return;
    pci_check_fun(bus, dev, 0);

    pci_print_device(bus, dev, 0);

    // device is multifunction, check other functions
    if (pci_get_header(bus, dev, 0).multifunction) 
        for (size_t fun = 1; fun < 8; fun++) {
            if (pci_get_vendor(bus, dev, fun) != INVALID_VENDOR) {
                pci_check_fun(bus, dev, fun);
                pci_print_device(bus, dev, fun);
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
