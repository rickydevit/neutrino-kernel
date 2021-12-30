#pragma once
#include "stdint.h"
#include "stdbool.h"

typedef struct display_info {
    uint64_t addr;
    uint16_t width;
    uint16_t height;
    uint16_t pitch;
    uint16_t bpp;
    uint8_t  red_mask_size;
    uint8_t  red_mask_shift;
    uint8_t  green_mask_size;
    uint8_t  green_mask_shift;
    uint8_t  blue_mask_size;
    uint8_t  blue_mask_shift;
} display_info_t;

bool init_video_driver(uint64_t* addr, uint16_t width, uint16_t height, uint16_t pitch, uint16_t bpp, 
                       uint8_t blue_mask_size, uint8_t red_mask_size, uint8_t green_mask_size,
                       uint8_t blue_mask_shift, uint8_t green_mask_shift, uint8_t red_mask_shift);
                       