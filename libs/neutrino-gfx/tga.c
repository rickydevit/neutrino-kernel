#include "tga.h"
#include <string.h>
#include <stdint.h>
#include <_null.h>
#include <liballoc.h>

// === PRIVATE FUNCTIONS ========================

// === PUBLIC FUNCTIONS =========================

void tga_get_pixel_array(TGAHeader* header, uintptr_t buf) {
    if (header == nullptr) return;
    if(header->h == 0 || header->w == 0 || header->bpp != 32) return;

    memset((uint8_t*)buf, 0, header->w*header->h*(header->bpp/8));
    memcpy(((uint8_t*)header) + sizeof(TGAHeader), (uint8_t*)buf, header->w*header->h*(header->bpp/8));
}
