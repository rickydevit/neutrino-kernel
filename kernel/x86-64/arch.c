#include "thirdparty/stivale2hdr.h"
#include "libs/libc/size_t.h"

void _kstart(struct stivale2_struct *stivale2_struct) {
    struct stivale2_struct_tag_terminal *term_str_tag;
    term_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_TERMINAL_ID);

    // If the terminal tag isn't found, halt the system.
    if (term_str_tag == NULL) for (;;) asm("hlt");

    void *term_write_ptr = (void *)term_str_tag->term_write;
    void (*term_write)(const char *string, size_t length) = term_write_ptr;

    term_write("Hello Limine", 12);

    for (;;) asm("hlt");
}