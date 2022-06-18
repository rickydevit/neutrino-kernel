#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <size_t.h>
#include <neutrino/macros.h>
#include <neutrino-gfx/color.h>

typedef struct __mask_shift {
    uint32_t red_mask, green_mask, blue_mask;
    uint8_t red_shift, green_shift, blue_shift;
} packed MaskShift;

typedef enum __display_depth {
    DISPLAY_BPP_4 = 0x04,
    DISPLAY_BPP_8 = 0x08,
    DISPLAY_BPP_15 = 0x0F,
    DISPLAY_BPP_16 = 0x10,
    DISPLAY_BPP_24 = 0x18,
    DISPLAY_BPP_32 = 0x20,
} DisplayDepth;

typedef struct __display_info {
    uintptr_t lbf;
    uint32_t width;
    uint32_t height;
    uint16_t pitch;
    DisplayDepth bpp;
} DisplayInfo;

bool init_video_driver(uintptr_t lbf, uint32_t width, uint32_t height, uint16_t pitch, DisplayDepth bpp);
void put_pixel(uint32_t pos_x, uint32_t pos_y, Color color);
void draw_rect(uint32_t pos_x, uint32_t pos_y, size_t width, size_t height, Color color);
