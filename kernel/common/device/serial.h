#pragma once
#include "port.h"
#include "libs/libc/string.h"

#define COM1 0x3f8
#define COM2 0x2f8
#define COM3 0x3e8
#define COM4 0x2e8

void init_serial(unsigned short port);
char serial_read();
void serial_write(char c);
void serial_write_string(char* message);
