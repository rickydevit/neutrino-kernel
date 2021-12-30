#pragma once
#include "gdt.h"
#include "memory/mem_virt.h"
#include "thirdparty/stivale2.h"
#include "stdint.h"

#define MAX_CPU 64
#define CPU_STACK_SIZE 0x2000

struct cpu {
    uint32_t id;                // CPU id
    uint32_t lapic_id;          // CPU lapic id

    uint8_t* stack;             // CPU stack
    uint8_t* stack_interrupt;   // CPU interrupt stack

    page_table* page_table;     // CPU page table physical address
    tss tss;
};

struct smp_t {
    uint32_t cpu_count;
    struct cpu cpus[MAX_CPU];
};

struct smp_t smp;

void init_smp(struct stivale2_struct_tag_smp*);
struct cpu* get_cpu(uint32_t id);
struct cpu* get_bootstrap_cpu();
void setup_bsp(uint8_t* bsp_stack);
