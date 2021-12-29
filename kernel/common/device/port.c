#include "port.h"

// *Read a byte (8 bit) from the given port
// @param port the port to read from
// @return the byte read from the port
unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    asm volatile("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

// *Write a byte to the given port
// @param port the port to write to
// @param data the data to write to the port
void port_byte_out(unsigned short port, unsigned char data) {
    asm volatile("out %%al, %%dx" : : "a" (data), "d" (port));
}

// *Read a word (16 bit) from the given port
// @port the port to read from
// @return the short read from the port
unsigned short port_word_in(unsigned short port) {
    unsigned short result;
    asm volatile("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

// *Write a word to the given port
// @param port the port to write to
// @param data the data to write to the port
void port_word_out(unsigned short port, unsigned short data) {
    asm volatile("out %%ax, %%dx" : : "a" (data), "d" (port));
}