#include "port.h"

// *Read a byte (8 bit) from the given port
// @param port the port to read from
// @return the byte read from the port
unsigned char portByteIn(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

// *Write a byte to the given port
// @param port the port to write to
// @param data the data to write to the port
void portByteOut(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

// *Read a word (16 bit) from the given port
// @port the port to read from
// @return the short read from the port
unsigned short portWordIn(unsigned short port) {
    unsigned short result;
    __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

// *Write a word to the given port
// @param port the port to write to
// @param data the data to write to the port
void portWordOut(unsigned short port, unsigned short data) {
    __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}