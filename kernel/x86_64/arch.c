#include "thirdparty/stivale2hdr.h"
#include "libs/libc/size_t.h"
#include "kernel/common/device/serial.h"
#include "kernel/common/video/display.h"

void _kstart(struct stivale2_struct *stivale2_struct) {
    struct stivale2_struct_tag_terminal *term_str_tag;
    struct stivale2_struct_tag_framebuffer *framebuf_str_tag;

    term_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_TERMINAL_ID);
    framebuf_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);

    // If the terminal tag isn't found, halt the system.
    // if (term_str_tag == NULL) for (;;) asm("hlt");
    if (framebuf_str_tag == NULL) for (;;) asm("hlt");

    if (init_video_driver(framebuf_str_tag->framebuffer_addr, framebuf_str_tag->framebuffer_width, framebuf_str_tag->framebuffer_height, 
                      framebuf_str_tag->framebuffer_pitch, framebuf_str_tag->framebuffer_bpp, framebuf_str_tag->red_mask_size, framebuf_str_tag->green_mask_size, 
                      framebuf_str_tag->blue_mask_size, framebuf_str_tag->red_mask_shift, framebuf_str_tag->green_mask_shift, framebuf_str_tag->blue_mask_shift))
        serial_write_string("Video driver initialized.");

    if (term_str_tag != NULL) {
        void *term_write_ptr = (void *)term_str_tag->term_write;
        void (*term_write)(const char *string, size_t length) = term_write_ptr;

        term_write("\n \
        _   _            _        _               _  __                    _  \n \
        | \\ | |          | |      (_)             | |/ /                   | | \n \
        |  \\| | ___ _   _| |_ _ __ _ _ __   ___   | ' / ___ _ __ _ __   ___| | \n \
        | . ` |/ _ \\ | | | __| '__| | '_ \\ / _ \\  |  < / _ \\ '__| '_ \\ / _ \\ | \n \
        | |\\  |  __/ |_| | |_| |  | | | | | (_) | | . \\  __/ |  | | | |  __/ | \n \
        |_| \\_|\\___|\\__,_|\\__|_|  |_|_| |_|\\___/  |_|\\_\\___|_|  |_| |_|\\___|_|", 461);
        term_write("v0.1-dev", 8);

        term_write("\n\n", 2);
        term_write("Hello world", 11);
    }

    for (;;) asm("hlt");
}
                                  
