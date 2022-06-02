#include "port.h"
#include <stdint.h>

// *Read a byte (8 bit) from the given port
// @param port the port to read from
// @return the byte read from the port
uint8_t port_byte_in(uint16_t port) {
    uint8_t result;
    asm volatile("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

// *Write a byte to the given port
// @param port the port to write to
// @param data the data to write to the port
void port_byte_out(uint16_t port, uint8_t data) {
    asm volatile("out %%al, %%dx" : : "a" (data), "d" (port));
}

// *Read a word (16 bit) from the given port
// @port the port to read from
// @return the short read from the port
uint16_t port_word_in(uint16_t port) {
    uint16_t result;
    asm volatile("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

// *Write a word to the given port
// @param port the port to write to
// @param data the data to write to the port
void port_word_out(uint16_t port, uint16_t data) {
    asm volatile("out %%ax, %%dx" : : "a" (data), "d" (port));
}

// *Read a double word (32 bit) from the given port
// @port the port to read from
// @return the short read from the port
uint32_t port_dword_in(uint16_t port) {
    uint32_t result;
    asm volatile("in %%dx, %%eax" : "=a" (result) : "d" (port));
    return result;
}

// *Write a double word to the given port
// @param port the port to write to
// @param data the data to write to the port
void port_dword_out(uint16_t port, uint32_t data) {
    asm volatile("out %%eax, %%dx" : : "a" (data), "d" (port));
}