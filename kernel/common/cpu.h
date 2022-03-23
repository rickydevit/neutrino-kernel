#pragma once

#include <stdint.h>
#include <size_t.h>
#include "tasks/task.h"

#define MAX_CPU 64
#define CPU_STACK_SIZE      0x2000
#define CPU_STACK_BASE      0xfffff80000000000

struct __tasks {
    Task* idle;
    Task* current;
    Task* next;
};

typedef struct __cpu Cpu;

#ifdef __x86_64
#include "kernel/x86_64/smp.h"
#else
#error "Unsupported platform"
#endif

size_t get_cpu_count();
Cpu* get_cpu(uint32_t id);
Cpu* get_current_cpu();
