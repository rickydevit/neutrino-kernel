#pragma once
#include "paging.h"
#include "libs/libc/stdint.h"

struct memory_virtual {
    uint8_t address_size; 
    page_table* active_page;
};

struct memory_virtual vmm;
