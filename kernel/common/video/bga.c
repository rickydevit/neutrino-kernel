#include "bga.h"
#include "display.h"
#include "../memory/memory.h"
#include "../device/pci/pci.h"
#include "../device/port.h"
#include <stdint.h>
#include <stdbool.h>
#include <_null.h>
#include "kservice.h"

static struct {
    size_t max_width, max_height;
    BGADisplayDepth max_depth;
    PCIDevice* pci_bga;
    size_t cur_width, cur_height;
    BGADisplayDepth cur_depth;
} bga_info = {0, 0, VBE_DISPI_BPP_4, nullptr, 0, 0, VBE_DISPI_BPP_4};

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
    bga_write(VBE_DISPI_INDEX_XRES, (width / 8 * 8) % bga_info.max_width);
    bga_write(VBE_DISPI_INDEX_YRES, height % bga_info.max_height);
    bga_write(VBE_DISPI_INDEX_BPP, (depth > VBE_DISPI_BPP_32) ? bga_info.max_depth : depth);
    bga_write(VBE_DISPI_INDEX_ENABLE, bga_read(VBE_DISPI_INDEX_ENABLE) | VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);

    bga_info.cur_depth = (depth > VBE_DISPI_BPP_32) ? bga_info.max_depth : depth;
    bga_info.cur_width = (width / 8 * 8) % bga_info.max_width;
    bga_info.cur_height = height % bga_info.max_height;
}

// === PUBLIC FUNCTIONS =========================

bool init_bga() {
    bga_info.pci_bga = pci_get_device_by_vendor(0x1234, 0x1111);
    if (bga_info.pci_bga == nullptr) return false;

    memory_map(bga_info.pci_bga->bars[0].range.base, bga_info.pci_bga->bars[0].range.base, bga_info.pci_bga->bars[0].range.size);

    // get max capabilities
    bga_write(VBE_DISPI_INDEX_ENABLE, bga_read(VBE_DISPI_INDEX_ENABLE) | VBE_DISPI_GETCAPS);
    bga_info.max_width = bga_read(VBE_DISPI_INDEX_XRES);
    bga_info.max_height = bga_read(VBE_DISPI_INDEX_YRES);
    bga_info.max_depth = bga_read(VBE_DISPI_INDEX_BPP);
    bga_write(VBE_DISPI_INDEX_ENABLE, bga_read(VBE_DISPI_INDEX_ENABLE) & ~VBE_DISPI_GETCAPS);
    
    bga_change_res(1280, 720, VBE_DISPI_BPP_24);

    ks.log("BGA - Max settings: %ux%ux%u Current settings: %ux%ux%u", 
        bga_info.max_width, bga_info.max_height, bga_info.max_depth, 
        bga_info.cur_width, bga_info.cur_height, bga_info.cur_depth);

    return true;
}

DisplayInfo bga_get_display_info() {
    return (DisplayInfo){
        .lbf = bga_info.pci_bga->bars[0].range.base, 
        .bpp = bga_info.cur_depth, 
        .pitch = bga_info.cur_width*(bga_info.cur_depth/8), 
        .height = bga_info.cur_height,
        .width = bga_info.cur_width
    };
}
