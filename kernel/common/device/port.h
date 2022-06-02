#pragma once

#ifdef __x86_64
#include "kernel/x86_64/device/port.h"
#else
#error "Unsupported platform"
#endif

extern unsigned char port_byte_in(unsigned short port);
extern void port_byte_out(unsigned short port, unsigned char data);
extern unsigned short port_word_in(unsigned short port);
extern void port_word_out(unsigned short port, unsigned short data);
