#include "pci.h"
#include "class_codes.h"
#include <size_t.h>
#include <liballoc.h>
#include <_null.h>
#include <linkedlist.h>
#include "kernel/common/memory/memory.h"
#include "kernel/common/device/port.h"
#include "kernel/common/kservice.h"

static List* device_list = nullptr;

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

void pci_write_config(uint8_t bus, uint8_t slot, uint8_t func, size_t offset, uint32_t data) {
    uint32_t address = ADDR_ENABLE | AddrBusID(bus) | AddrDevID(slot) | AddrFunID(func) | (offset & 0xfc);
    port_dword_out(CONFIG_ADDR_PORT, address);
    port_dword_out(CONFIG_DATA_PORT, data);
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

size_t pci_get_bar_size(uint8_t bus, uint8_t dev, uint8_t fun, BaseAddrReg bar_ptr[], size_t i) {
    if (bar_ptr[i].bar == 0) return 0;
    volatile uintptr_t saved_addr = GetBARAddr(bar_ptr, 0);
    size_t offset = BAR_LIST_OFFSET + (i*4);
    size_t size = 0;

    if (bar_ptr[i].bar_type == BAR_MEMORY_SPACE /*&& 
    bar_ptr[i].memory_bar_type == MEMORY_BAR_64BIT */) {
        // write 1s
        pci_write_config(bus, dev, fun, offset, 0xffffffff);
        
        // read size
        port_dword_out(CONFIG_ADDR_PORT, ADDR_ENABLE | AddrBusID(bus) | AddrDevID(dev) | AddrFunID(fun) | (offset & 0xfc));
        size = ~(port_dword_in(CONFIG_DATA_PORT) & 0xfffffff0) + 1;

        // restore
        pci_write_config(bus, dev, fun, offset, saved_addr);
    }

    return size;
}

size_t pci_get_bar_count(BaseAddrReg bar_ptr[], PCIHeaderType hdr_type)  {
    size_t i = 0, size = 0;
    while (i<(hdr_type.type == 0x0 ? 6 : 2)) {
        if (bar_ptr[i].bar == 0) break;
        size++;
        i += (bar_ptr[i].bar_type == BAR_MEMORY_SPACE && 
              bar_ptr[i].memory_bar_type == MEMORY_BAR_64BIT) ? 2 : 1;
    };

    return size;
}

void pci_print_device(PCIDevice* d) {
    ks.dbg("PCI - (%u:%u.%u) device %x:%x class: %c - subclass: %c", d->location.bus, d->location.device, d->location.function, d->dev_info.vendor_id, 
        d->dev_info.device_id, pci_get_class_string(d->class), pci_get_subclass_string(d->class, d->subclass));

    if (d->bars_count > 0 && d->bars != nullptr) {
        for (size_t i = 0; i < d->bars_count; i++) {
            ks.dbg("      BAR%u: %c space at %x of size %x", i, (d->bars[i].type == BAR_MEMORY_SPACE) ? "memory" : "io", 
                    d->bars[i].range.base, d->bars[i].range.size); 
        }
    }
}

PCIDevice* pci_add_device(PCILocation loc, PCIConfigDataCommon* common) {
    PCIDevice* ndev = (PCIDevice*)kmalloc(sizeof(PCIDevice));
    ndev->class = common->class_code;
    ndev->subclass = common->subclass;
    ndev->dev_info.device_id = common->device;
    ndev->dev_info.vendor_id = common->vendor;
    ndev->location = loc;
    ndev->bars_count = 0;
    ndev->bars = nullptr;

    if (common->header_type.type == 0x0 || common->header_type.type == 0x1) {
        PCIConfigGeneralDevice* h = (PCIConfigGeneralDevice*)common;
        ndev->bars_count = pci_get_bar_count(h->base_addr, h->common.header_type);
        ndev->bars = (PCIBarInfo*)kmalloc(sizeof(PCIBarInfo) * ndev->bars_count);
        
        for (size_t i = 0; i < ndev->bars_count; i++) {
            ndev->bars[i].type = h->base_addr[i].bar_type;
            ndev->bars[i].range.base = GetBARAddr(h->base_addr, i);
            ndev->bars[i].range.size = pci_get_bar_size(loc.bus, loc.device, loc.function, h->base_addr, i);
        }
    }

    device_list = list_append(device_list, ndev);
    return ndev;
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

    pci_print_device(pci_add_device((PCILocation){bus, dev, 0}, pci_read_full_config(bus, dev, 0)));

    // device is multifunction, check other functions
    if (pci_get_header(bus, dev, 0).multifunction) 
        for (size_t fun = 1; fun < 4; fun++) {
            if (pci_get_vendor(bus, dev, fun) != INVALID_VENDOR) {
                pci_check_fun(bus, dev, fun);
                pci_print_device(pci_add_device((PCILocation){bus, dev, fun}, pci_read_full_config(bus, dev, fun)));
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

PCIDevice* pci_get_device_by_class(uint8_t class, uint8_t subclass) {
    List* p = device_list;
    PCIDevice* dev = nullptr;

    while (p != nullptr) {
        dev = list_get_value(p);

        if (dev->class == class && dev->subclass == subclass)
            return dev;

        p = p->next;
    }

    return nullptr;
}

PCIDevice* pci_get_device_by_vendor(uint16_t vendor_id, uint16_t device_id) {
    List* p = device_list;
    PCIDevice* dev = nullptr;

    while (p != nullptr) {
        dev = list_get_value(p);

        if (dev->dev_info.vendor_id == vendor_id && 
        dev->dev_info.device_id == device_id)
            return dev;

        p = p->next;
    }

    return nullptr;
}

// ===  PUBLIC FUNCTIONS ========================

void init_pci() {
    pci_check_all_buses();
    ks.log("PCI devices enumeration complete");
}
