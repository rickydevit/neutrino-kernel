#include "display.h"
#include "libs/libc/stdbool.h"
#include "libs/libc/stdint.h"
#include "libs/_null.h"

display_info_t info;

// === PUBLIC FUNCTIONS =================================

// *Initialize the video driver given the correct parameters
// @param addr the address of the VRAM location
// @param width the width of the display
// @param height the height of the display
// @param pitch the bytes to move one pixel downwards
// @param bpp the bits per pixel, how many bits does each pixel use
// @param blue_mask_size the size of the blue channel mask
// @param red_mask_size the size of the red channel mask
// @param green_mask_size the size of the green channel mask
// @param blue_mask_shift the shift in bits of the blue channel mask
// @param green_mask_shift the shift in bits of the green channel mask
// @param red_mask_shift the shift in bits of the red channel mask
// @return true if the driver is initialized correctly, false otherwise
bool init_video_driver(uint64_t* addr, uint16_t width, uint16_t height, uint16_t pitch, uint16_t bpp, 
                       uint8_t red_mask_size, uint8_t green_mask_size, uint8_t blue_mask_size, 
                       uint8_t red_mask_shift, uint8_t green_mask_shift, uint8_t blue_mask_shift) {
   
    if (width <= 0 || height <= 0 || pitch <= 0 || bpp <= 0) return false;

    info.addr = (uint64_t)addr;
    info.width = width;
    info.height = height;
    info.pitch = pitch;
    info.bpp = bpp;
    info.blue_mask_size = blue_mask_size;
    info.green_mask_size = green_mask_size;
    info.red_mask_size = red_mask_size;
    info.blue_mask_shift = blue_mask_shift;
    info.green_mask_shift = green_mask_shift;
    info.red_mask_shift = red_mask_shift;

    return true;
}

// === PRIVATE FUNCTIONS ================================

void put_pixel(uint32_t pos_x, uint32_t pos_y, uint32_t color) {
    uint32_t* pixel = (uint32_t*)(info.addr + pos_y*info.pitch + pos_x*(info.bpp/8));
    *(pixel) = ((color >> 16 & 0xff) << info.red_mask_shift) | 
               ((color >> 8 & 0xff) << info.green_mask_shift) | 
               ((color & 0xff) << info.blue_mask_shift);
}


