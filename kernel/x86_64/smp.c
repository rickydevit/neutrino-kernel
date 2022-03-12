#include "smp.h"
#include "gdt.h"
#include "arch.h"
#include "interrupts.h"
#include "device/apic.h"
#include "device/time/hpet.h"
#include "memory/mem_virt.h"
#include "memory/mem_phys.h"
#include "kernel/common/kservice.h"
#include <limine/stivale2.h>
#include <_null.h>
#include <stdbool.h>

static volatile bool cpu_started = false;

// === PRIVATE FUNCTIONS ========================

void start_cpu(struct stivale2_smp_info* smp_info) {
    disable_interrupts();

    init_gdt_on_ap(smp_info->processor_id);
    init_idt();

    init_vmm_on_ap(smp_info);
    init_tss(get_cpu(smp_info->processor_id));
    
    init_sse();
    map_apic_on_ap();
    enable_apic();

    if (has_hpet()) vmm_map_mmio(get_rmmio_address(hpet.base), 1);
    init_apic_timer();

    cpu_started = true;

    enable_interrupts();
    for (;;) asm volatile("hlt");
}

// === PUBLIC FUNCTIONS =========================

void init_smp(struct stivale2_struct_tag_smp *smp_struct) {
    ks.log("Initializing other CPUs...");
    ks.dbg("Found %i CPUs. x2APIC is %c", smp_struct->cpu_count, smp_struct->flags & 1 ? "enabled" : "disabled");

    apic.x2apic_enabled = smp_struct->flags & 1;
    smp.cpu_count = (smp_struct->cpu_count > MAX_CPU ? MAX_CPU : smp_struct->cpu_count);

    // save cpus
    for (int i = 0; i < smp.cpu_count; i++) {
        struct stivale2_smp_info cpu_info = smp_struct->smp_info[i];
        ks.dbg("cpu id: %d lapic id: %d", cpu_info.processor_id, cpu_info.lapic_id);
        smp.cpus[i].id = cpu_info.processor_id;
        smp.cpus[i].lapic_id = cpu_info.lapic_id;

        // setup the tss
        smp.cpus[i].tss.iopb_offset = sizeof(Tss);
        smp.cpus[i].tss.rsp0 = (uint64_t)smp.cpus[i].stack;
        smp.cpus[i].stack_interrupt = pmm_alloc_series(CPU_STACK_SIZE / PHYSMEM_BLOCK_SIZE);
        smp.cpus[i].tss.ist1 = (uint64_t)smp.cpus[i].stack_interrupt + CPU_STACK_SIZE;

        // vmm_map_page(smp.cpus[i].stack_interrupt, 1);

        // skip the bsp
        if (cpu_info.lapic_id == smp_struct->bsp_lapic_id) continue;

        // prepare the stack
        smp.cpus[i].stack = pmm_alloc_series(CPU_STACK_SIZE/PHYSMEM_BLOCK_SIZE);
        smp_struct->smp_info[i].target_stack = get_mem_address(smp.cpus[i].stack) + CPU_STACK_SIZE;
        smp.cpus[i].tss.rsp0 = (uint64_t)smp.cpus[i].stack;

        // boot the ap
        ks.log("Starting CPU #%d, stack at %x, trampoline at %x", i, smp_struct->smp_info[i].target_stack, (uint64_t)start_cpu);
        smp_struct->smp_info[i].goto_address = (uint64_t)start_cpu;
        
        // wait for ap to boot up
        while (!cpu_started);
        ks.log("CPU #%d started successfully", i);
        cpu_started = false;
    }

    ks.log("%u CPUs initialized successfully.", smp.cpu_count);
    disable_interrupts();
}

// *Set the information about the BSP on startup
// @param bsp_stack the stack of the bootstrap processor
void setup_bsp(uint8_t* bsp_stack) {
    smp.cpus[0].id = 0;
    smp.cpus[0].lapic_id = 0;
    smp.cpus[0].stack = bsp_stack;
}

// *Get the cpu info given the cpu id
// @param id the cpu id to get the info from
// @return the pointer to the cpu info structure
Cpu* get_cpu(uint32_t id) {
    if (id != 0 && id >= smp.cpu_count) {
        ks.warn("Cannot find cpu with id %u", id);
        return (Cpu*)NULL;
    }

    return &(smp.cpus[id]);
}

// *Get the cpu info about the boostrap processor
// @return the pointer to the cpu info structure of the boostrap processor
Cpu* get_bootstrap_cpu() {
    return get_cpu(0);
}

// *Get the cpu info about the current processor
// @return the pointer to the cpu info structure of the current processor
Cpu* get_current_cpu() {
    uint32_t id = LapicIDCorrection(apic_read(lapic_id));

    for (uint32_t i = 0; i < smp.cpu_count; i++) {
        if (smp.cpus[i].lapic_id == id)
            return &(smp.cpus[i]);
    }

    ks.warn("Cannot find cpu with lapic id %u", id);
    return (Cpu*)NULL;
}
