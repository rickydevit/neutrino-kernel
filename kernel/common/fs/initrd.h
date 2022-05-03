#pragma once
#include <stdint.h>
#include "fs.h"

typedef struct __initrd_header {
    uint32_t n_files;
} InitrdHeader;

typedef struct __initrd_file_header {
    uint8_t magic;
    char name[64];
    uintptr_t offset;
    size_t length;
} InitrdFileHeader;

FsNode* init_initrd(uintptr_t addr);
