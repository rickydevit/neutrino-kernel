#include "smp.h"
#include "gdt.h"
#include "arch.h"
#include "apic.h"
#include "memory/mem_virt.h"
#include "memory/mem_phys.h"
#include "kernel/common/kservice.h"
#include "thirdparty/stivale2.h"
#include "libs/libc/stdbool.h"

static volatile bool cpu_started = false;

// === PRIVATE FUNCTIONS ========================

void start_cpu(struct stivale2_smp_info* smp_info) {
    init_gdt();
    init_idt();
    init_vmm_on_ap(smp_info);
    
    init_sse();
    // enable_apic();

    cpu_started = true;

    asm("sti");
    for (;;) asm("hlt");
}

// === PUBLIC FUNCTIONS =========================

void init_smp(struct stivale2_struct_tag_smp *smp_struct) {
    ks.log("Initializing other CPUs...");
    ks.dbg("Found %i CPUs. x2APIC is %c", smp_struct->cpu_count, smp_struct->flags & 1 ? "enabled" : "disabled");

    smp.cpu_count = (smp_struct->cpu_count > MAX_CPU ? MAX_CPU : smp_struct->cpu_count);

    // save cpus
    for (int i = 0; i < smp.cpu_count; i++) {
        struct stivale2_smp_info cpu_info = smp_struct->smp_info[i];
        ks.dbg("cpu id: %d lapic id: %d", cpu_info.processor_id, cpu_info.lapic_id);
        smp.cpus[i].id = cpu_info.processor_id;
        smp.cpus[i].lapic_id = cpu_info.lapic_id;

        // skip the bsp
        if (cpu_info.lapic_id == smp_struct->bsp_lapic_id) continue;

        // prepare the stack
        smp.cpus[i].stack = pmm_alloc_series(CPU_STACK_SIZE/PHYSMEM_BLOCK_SIZE);
        smp_struct->smp_info[i].target_stack = get_mem_address(smp.cpus[i].stack) + CPU_STACK_SIZE;

        // boot the ap
        ks.log("Starting CPU #%d, stack at %x, trampoline at %x", i, smp_struct->smp_info[i].target_stack, (uint64_t)start_cpu);
        smp_struct->smp_info[i].goto_address = (uint64_t)start_cpu;
        
        // wait for ap to boot up
        while (!cpu_started);
        ks.log("CPU #%d started successfully", i);
        cpu_started = false;
    }

    ks.log("%u CPUs initialized successfully.", smp.cpu_count);
}

// *Get the cpu info given the cpu id
// @param id the cpu id to get the info from
// @return the pointer to the cpu info structure
struct cpu* get_cpu(uint32_t id) {
    if (id != 0 && id >= smp.cpu_count) {
        ks.warn("Cannot find cpu with id %u", id);
        return (struct cpu*)0x0;
    }
    return &(smp.cpus[id]);
}

// *Get the cpu info about the boostrap processor
// @return the pointer to the cpu info structure of the boostrap processor
struct cpu* get_bootstrap_cpu() {
    return get_cpu(0);
}
