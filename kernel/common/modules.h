#pragma once
#include <stdint.h>
#include <size_t.h>

#define MAX_NAME 64
#define MAX_MODULES 32

typedef struct __boot_module {
    char name[MAX_NAME];
    uintptr_t start_addr;
    size_t length;
} BootModule;

int init_modules(uintptr_t);
BootModule* module_get_by_name(const char*);

#ifdef __limine
#include "libs/limine/modules.h"
#else
#error "Unsupported bootloader"
#endif

