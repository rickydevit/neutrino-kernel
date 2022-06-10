#include "arch.h"
#include "cpuid.h"
#include "interrupts.h"
#include "gdt.h"
#include "smp.h"
#include "sse.h" 
#include "pic.h"
#include "device/apic.h"
#include "memory/mem_virt.h"
#include "memory/mem_phys.h"
#include "device/acpi.h"
#include "device/time/hpet.h"
#include "device/time/rtc.h"
#include "kernel/common/device/serial.h"
#include "kernel/common/kservice.h"
#include "kernel/common/tasks/scheduler.h"
#include "kernel/common/modules.h"
#include "kernel/common/neutrino.h"
#include "syscall.h"
#include <size_t.h>
#include <_null.h>
#include <libs/limine/stivale2hdr.h>
#include <neutrino/macros.h>
#include <neutrino/time.h>

void kinit_mem_manager(struct stivale2_struct_tag_memmap* memmap_str_tag, MemoryPhysicalRegion* entries) {
    uint32_t memmap_entries = memmap_str_tag->entries;
    int i = 0, lookahead = 1;
    
    while (i < memmap_entries) {
        struct stivale2_mmap_entry *entry = memmap_str_tag->memmap + i;

        entries[i].base = entry->base;
        entries[i].size = entry->length;
        entries[i].limit = entry->base + entry->length;
        
        if (entry->type == STIVALE2_MMAP_USABLE) entries[i].type = MEMORY_REGION_USABLE;
        else if (entry->type == STIVALE2_MMAP_KERNEL_AND_MODULES) entries[i].type = MEMORY_REGION_KERNEL;
        else if (entry->type == STIVALE2_MMAP_FRAMEBUFFER) entries[i].type = MEMORY_REGION_FRAMEBUFFER;
        else if (entry->type == STIVALE2_MMAP_ACPI_NVS) entries[i].type = MEMORY_REGION_ACPI_RSVD;
        else if (entry->type == STIVALE2_MMAP_ACPI_RECLAIMABLE) entries[i].type = MEMORY_REGION_ACPI_RCLM;
        else entries[i].type = MEMORY_REGION_RESERVED;

        if (i+lookahead < memmap_entries) {
            while ((entries[i].type == MEMORY_REGION_USABLE && (entry+lookahead)->type == STIVALE2_MMAP_USABLE)) {
                        entries[i].size += (entry+lookahead)->length;
                        entries[i].limit = (entry+lookahead)->base + (entry+lookahead)->length;
                        entries[i+lookahead].type = MEMORY_REGION_INVALID;
                        lookahead++;
            }
            i += lookahead;
            lookahead = 1;
        } else {
            i++;
        }
    }

    init_pmm(entries, memmap_entries);
    init_vmm();
}

void unoptimized _kstart(struct stivale2_struct *stivale2_struct) {
    struct stivale2_struct_tag_memmap *memmap_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);
    struct stivale2_struct_tag_smp *smp_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_SMP_ID);
    struct stivale2_struct_tag_modules *modules = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MODULES_ID);
    MemoryPhysicalRegion entries[memmap_str_tag->entries];

    disable_interrupts();

    //? Core initialization
    setup_bsp(stack);
    init_serial(COM1);
    init_kservice();
    init_gdt();
    init_idt();
    init_cpuid();

    kinit_mem_manager(memmap_str_tag, entries);

    init_tss(get_bootstrap_cpu());
    init_pic();
    init_sse();
    init_acpi();
    init_apic();
    init_hpet();
    init_apic_timer();
    init_smp(smp_str_tag);
    init_modules((uintptr_t)modules);
    init_scheduler();    
    init_syscall();

    Task* neutrino = NewTask("neutrino", false);
    sched_start(neutrino, (uintptr_t)neutrino_main);

    // arch-dependant _kstart() is done setting up the system with the right architecture.
    // flow is passed to neutrino_main() in "kernel/common" to continue the set up process. see ya!
    enable_interrupts();

    //? -----------------------------------------
    ks.fatal(FatalError(FATAL_ERROR, "Neutrino returned!"));
    for (;;) asm volatile("hlt");
}

void arch_idle() {
    while (true) asm volatile ("hlt");
}

Timestamp arch_now() {
    return datetime_to_timestamp(cmos_read());
}
