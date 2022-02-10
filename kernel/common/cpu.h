#pragma once

#include <stdint.h>
#include "tasks/task.h"

#ifdef __x86_64
#include "kernel/x86_64/smp.h"
#else
#error "Unsupported platform"
#endif

#define MAX_CPU 64

struct __tasks {
    Task* idle;
    Task* current;
    Task* next;
};

typedef struct __cpu Cpu;

Cpu* get_cpu(uint32_t id);
Cpu* get_current_cpu();
