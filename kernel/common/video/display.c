#include "display.h"
#include <stdbool.h>
#include <stdint.h>
#include <_null.h>
#include <neutrino-gfx/color.h>

static DisplayInfo info;
static const MaskShift mask_shift[] = {
    [DISPLAY_BPP_15] = {(0x1f << 10), (0x1f << 5), (0x1f << 0), 10, 5, 0},
    [DISPLAY_BPP_16] = {(0x1f << 11), (0x3f << 5), (0x1f << 0), 11, 5, 0},
    [DISPLAY_BPP_24] = {(0xff << 16), (0xff << 8), (0xff << 0), 16, 8, 0},
    [DISPLAY_BPP_32] = {(0xff << 16), (0xff << 8), (0xff << 0), 16, 8, 0}
};

// === PUBLIC FUNCTIONS =================================

// *Initialize the video driver given the correct parameters
// @param addr the address of the VRAM location
// @param width the width of the display
// @param height the height of the display
// @param pitch the bytes to move one pixel downwards
// @param bpp the bits per pixel, how many bits does each pixel use
// @return true if the driver is initialized correctly, false otherwise
bool init_video_driver(uintptr_t lbf, uint32_t width, uint32_t height, uint16_t pitch, DisplayDepth bpp) {
    if (width <= 0 || height <= 0 || bpp <= 0) return false;

    info.lbf = lbf;
    info.width = width;
    info.height = height;
    info.pitch = pitch;
    info.bpp = bpp;

    return true;
}

// === PRIVATE FUNCTIONS ================================

void put_pixel(uint32_t pos_x, uint32_t pos_y, Color color) {
    uintptr_t pixel = (uintptr_t)(info.lbf + pos_y*info.pitch + pos_x*(info.bpp/8));

    *(uint32_t*)(pixel) = ((color.red << mask_shift[info.bpp].red_shift) & mask_shift[info.bpp].red_mask) | 
               ((color.green << mask_shift[info.bpp].green_shift) & mask_shift[info.bpp].green_mask) | 
               ((color.blue << mask_shift[info.bpp].blue_shift) & mask_shift[info.bpp].blue_mask);
}
