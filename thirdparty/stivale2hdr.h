#pragma once
#include "libs/_null.h"
#include "libs/libc/stdint.h"
#include "stivale2.h"

#define CHECKER_NEITHER_AVAILABLE       0
#define CHECKER_FRAMEBUFFER_AVAILABLE   1
#define CHECKER_TERMINAL_AVAILABLE      2

// ?stack for the limine bootloader
static uint8_t stack[8192];

static struct stivale2_tag unmap_null_hdr_tag = {
    .identifier = STIVALE2_HEADER_TAG_UNMAP_NULL_ID,
    .next = 0,
};

static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // Same as above.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        // Instead of 0, we now point to the previous header tag. The order in
        // which header tags are linked does not matter.
        // .next = (uint64_t)&terminal_hdr_tag
        .next = (uint64_t)&unmap_null_hdr_tag
    },
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0
};

static struct stivale2_header_tag_any_video any_video_hdr_tag = {
        // Same as above.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_ANY_VIDEO_ID,
        // Instead of 0, we now point to the previous header tag. The order in
        // which header tags are linked does not matter.
        .next = (uint64_t)&framebuffer_hdr_tag
    },
    .preference = 0
};

__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
    // The entry_point member is used to specify an alternative entry
    // point that the bootloader should jump to instead of the executable's
    // ELF entry point. We do not care about that so we leave it zeroed.
    .entry_point = 0,
    // Let's tell the bootloader where our stack is.
    // We need to add the sizeof(stack) since in x86(_64) the stack grows
    // downwards.
    .stack = (uintptr_t)stack + sizeof(stack),
    // Bit 1, if set, causes the bootloader to return to us pointers in the
    // higher half, which we likely want.
    // Bit 2, if set, tells the bootloader to enable protected memory ranges,
    // that is, to respect the ELF PHDR mandated permissions for the executable's
    // segments.
    .flags = (1 << 1) | (1 << 2),
    // This header structure is the root of the linked list of header tags and
    // points to the first one in the linked list.
    .tags = (uintptr_t)&any_video_hdr_tag
};

void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id) {
    struct stivale2_tag *current_tag = (void *)stivale2_struct->tags;
    for (;;) {
        // If the tag pointer is NULL (end of linked list), we did not find
        // the tag. Return NULL to signal this.
        if (current_tag == NULL) return NULL;
 
        // Check whether the identifier matches. If it does, return a pointer
        // to the matching tag.
        if (current_tag->identifier == id) {
            return current_tag;
        }
 
        // Get a pointer to the next tag in the linked list and repeat.
        current_tag = (void *)current_tag->next;
    }
}

// *Check if the limine loader returned either a valid framebuffer struct tag or a valid terminal struct tag
// @param framebuffer_addr the linear framebuffer struct tag address
// @param terminal_addr the terminal struct tag address
// @return true if either one of the struct tags is valid, false otherwise
uint32_t check_framebuffer_or_terminal(void* framebuffer_addr, void* terminal_addr) {
    if (framebuffer_addr != NULL) return CHECKER_FRAMEBUFFER_AVAILABLE;
    if (terminal_addr != NULL) return CHECKER_TERMINAL_AVAILABLE;

    return CHECKER_NEITHER_AVAILABLE;
}
