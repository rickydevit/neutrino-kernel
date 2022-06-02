#pragma once
#include <stdint.h>

#ifdef __x86_64
#include "kernel/x86_64/device/port.h"
#else
#error "Unsupported platform"
#endif

uint8_t port_byte_in(uint16_t port);
void port_byte_out(uint16_t port, uint8_t data);
uint16_t port_word_in(uint16_t port);
void port_word_out(uint16_t port, uint16_t data);
uint32_t port_dword_in(uint16_t port);
void port_dword_out(uint16_t port, uint32_t data);