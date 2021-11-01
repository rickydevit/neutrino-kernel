#include "memory.h"
#include "libs/libc/stdint.h"

// *Copy [nbytes] bytes from the [source] memory pointer to the [dest] memory pointer
// @param source the source memory pointer
// @param dest the destination memory pointer
// @param nbytes the number of bytes to copy
void memory_copy(uint8_t *source, uint8_t *dest, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        *(dest + i) = *(source + i);
    }
}

// *Set [len] bytes at the [dest] memory location to the [val] value
// @param dest the memory location to set bytes
// @param val the value to set the bytes to
// @param len the number of bytes to set
void memory_set(uint8_t *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}
