#pragma once
#include <stdbool.h>

typedef enum __bga_io_port {
    VBE_DISPI_IOPORT_INDEX = 0x01ce,
    VBE_DISPI_IOPORT_DATA = 0x01cf,
} BGAIOPort;

typedef enum __bga_io_port_index {
    VBE_DISPI_INDEX_ID,
    VBE_DISPI_INDEX_XRES,
    VBE_DISPI_INDEX_YRES,
    VBE_DISPI_INDEX_BPP,
    VBE_DISPI_INDEX_ENABLE,
    VBE_DISPI_INDEX_BANK,
    VBE_DISPI_INDEX_VIRT_WIDTH,
    VBE_DISPI_INDEX_VIRT_HEIGHT,
    VBE_DISPI_INDEX_X_OFFSET,
    VBE_DISPI_INDEX_Y_OFFSET,
} BGAIOPortIndex;

typedef enum __bga_display_depth {
    VBE_DISPI_BPP_4 = 0x04,
    VBE_DISPI_BPP_8 = 0x08,
    VBE_DISPI_BPP_15 = 0x0F,
    VBE_DISPI_BPP_16 = 0x10,
    VBE_DISPI_BPP_24 = 0x18,
    VBE_DISPI_BPP_32 = 0x20,
} BGADisplayDepth;

typedef enum __bga_enable_settings {
    VBE_DISPI_DISABLED = 0x00,
    VBE_DISPI_ENABLED = 0x01,
    VBE_DISPI_GETCAPS = 0x02,
    VBE_DISPI_8BIT_DAC = 0x20,
    VBE_DISPI_LFB_ENABLED = 0x40,
    VBE_DISPI_NOCLEARMEM = 0x80,
} BGAEnableSettings;

bool init_bga();
