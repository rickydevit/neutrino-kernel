#pragma once
#include "kernel/common/cpu.h"
#include "gdt.h"
#include "memory/mem_virt.h"
#include <limine/stivale2.h>
#include <stdint.h>

#define CPU_STACK_SIZE 0x2000

struct __cpu {
    uint32_t id;                // CPU id
    uint32_t lapic_id;          // CPU lapic id

    uint8_t* stack;             // CPU stack
    uint8_t* stack_interrupt;   // CPU interrupt stack

    PageTable* page_table;     // CPU page table physical address
    Tss tss;

    struct __tasks tasks;
};

struct smp_t {
    uint32_t cpu_count;
    Cpu cpus[MAX_CPU];
};

struct smp_t smp;

void init_smp(struct stivale2_struct_tag_smp*);
Cpu* get_cpu(uint32_t id);
Cpu* get_current_cpu();
Cpu* get_bootstrap_cpu();
void setup_bsp(uint8_t* bsp_stack);