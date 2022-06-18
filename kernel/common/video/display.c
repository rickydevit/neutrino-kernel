#include "display.h"
#include <stdbool.h>
#include <stdint.h>
#include <size_t.h>
#include <_null.h>
#include <neutrino-gfx/color.h>
#include <neutrino/macros.h>

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
    if (width <= 0 || height <= 0 || bpp != DISPLAY_BPP_32) return false;

    info.lbf = lbf;
    info.width = width;
    info.height = height;
    info.pitch = pitch;
    info.bpp = bpp;

    return true;
}

// === PRIVATE FUNCTIONS ================================

Color color_blend(Color back, Color top) {
    uint32_t alpha = top.alpha + 1;
    uint32_t inv_alpha = 256 - top.alpha;

    if (top.alpha == 0) return back;

    return (Color){
        .red = (uint8_t)((alpha * top.red + inv_alpha * back.red) >> 8),
        .green = (uint8_t)((alpha * top.green + inv_alpha * back.green) >> 8),
        .blue = (uint8_t)((alpha * top.blue + inv_alpha * back.blue) >> 8),
        .alpha = 0xff
    };
}

void unoptimized put_pixel(uint32_t pos_x, uint32_t pos_y, Color color) {
    uintptr_t pixel = (uintptr_t)(info.lbf + pos_y*info.pitch + pos_x*(info.bpp/8));

    uint32_t dest = *(volatile uint32_t*)(pixel);
    Color alpha_corrected = color_blend((Color){
        .red = (dest & mask_shift[info.bpp].red_mask) >>  mask_shift[info.bpp].red_shift,
        .green = (dest & mask_shift[info.bpp].green_mask) >> mask_shift[info.bpp].green_shift,
        .blue = (dest & mask_shift[info.bpp].blue_mask) >> mask_shift[info.bpp].blue_shift,
        .alpha = 0xff
    }, color);

    *(uint32_t*)(pixel) = (((alpha_corrected.red << mask_shift[info.bpp].red_shift) & mask_shift[info.bpp].red_mask) | 
               ((alpha_corrected.green << mask_shift[info.bpp].green_shift) & mask_shift[info.bpp].green_mask) | 
               ((alpha_corrected.blue << mask_shift[info.bpp].blue_shift) & mask_shift[info.bpp].blue_mask));
}

void draw_rect(uint32_t pos_x, uint32_t pos_y, size_t width, size_t height, Color color) {
    uintptr_t loc = info.lbf + (pos_y*info.pitch);
    
    for (size_t i = 0; i < height; i++) {
        for (size_t j = pos_x; j < pos_x + width; j++) {
            *(uint32_t*)(loc + j*(info.bpp/8)) = ((color.red << mask_shift[info.bpp].red_shift) & mask_shift[info.bpp].red_mask) | 
               ((color.green << mask_shift[info.bpp].green_shift) & mask_shift[info.bpp].green_mask) | 
               ((color.blue << mask_shift[info.bpp].blue_shift) & mask_shift[info.bpp].blue_mask);
        }

        loc += info.pitch;
    }
}
