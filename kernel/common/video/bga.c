#include "bga.h"
#include "../device/pci/pci.h"
#include "../device/port.h"
#include <stdint.h>
#include <stdbool.h>
#include <_null.h>
#include "kservice.h"

static struct {
    size_t max_width, max_height;
    BGADisplayDepth max_depth;
} bga_max_settings = {0, 0, VBE_DISPI_BPP_4};

// === PRIVATE FUNCTIONS ========================

void bga_write(BGAIOPortIndex index, uint16_t data) {
    port_word_out(VBE_DISPI_IOPORT_INDEX, index);
    port_word_out(VBE_DISPI_IOPORT_DATA, data);
}

uint16_t bga_read(BGAIOPortIndex index) {
    port_word_out(VBE_DISPI_IOPORT_INDEX, index);
    return port_word_in(VBE_DISPI_IOPORT_DATA);
}

void bga_change_res(size_t width, size_t height, BGADisplayDepth depth) {
    bga_write(VBE_DISPI_INDEX_ENABLE, bga_read(VBE_DISPI_INDEX_ENABLE) & ~VBE_DISPI_ENABLED);
    bga_write(VBE_DISPI_INDEX_XRES, (width / 8 * 8) % bga_max_settings.max_width);
    bga_write(VBE_DISPI_INDEX_YRES, height % bga_max_settings.max_height);
    bga_write(VBE_DISPI_INDEX_BPP, (depth > VBE_DISPI_BPP_32) ? bga_max_settings.max_depth : depth);
    bga_write(VBE_DISPI_INDEX_ENABLE, bga_read(VBE_DISPI_INDEX_ENABLE) | VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
}

// === PUBLIC FUNCTIONS =========================

bool init_bga() {
    PCIDevice* pci_bga = pci_get_device_by_vendor(0x1234, 0x1111);
    if (pci_bga == nullptr) return false;

    // get max capabilities
    bga_write(VBE_DISPI_INDEX_ENABLE, bga_read(VBE_DISPI_INDEX_ENABLE) | VBE_DISPI_GETCAPS);
    bga_max_settings.max_width = bga_read(VBE_DISPI_INDEX_XRES);
    bga_max_settings.max_height = bga_read(VBE_DISPI_INDEX_YRES);
    bga_max_settings.max_depth = bga_read(VBE_DISPI_INDEX_BPP);
    bga_write(VBE_DISPI_INDEX_ENABLE, bga_read(VBE_DISPI_INDEX_ENABLE) & ~VBE_DISPI_GETCAPS);

    ks.log("Max width: %u, max height: %u, max depth: %u", bga_max_settings.max_width, 
            bga_max_settings.max_height, bga_max_settings.max_depth);

    bga_change_res(1280, 720, VBE_DISPI_BPP_24);

    return true;
}
