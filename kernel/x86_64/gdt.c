#include "gdt.h"
#include "smp.h"
#include "kservice.h"
#include "memory/mem_phys.h"
#include "memory/mem_virt.h"

struct cpu_GDT gdt_array[MAX_CPU];

// === PRIVATE FUNCTIONS ========================

// *Setup the GDT with the base descriptors
// @param gdt the gdt pointer to the given cpu's gdt entry
void setup_gdt(struct cpu_GDT* gdt) {
    // Null descriptor 0x0
    gdt->GDT[0] = gdt_entry_create(0, 0, 0, 0);

    // 32-bit code and data descriptors 0x08 0x10
    gdt->GDT[1] = gdt_entry_create(0, 0xffffffff, GDT_PRESENT | GDT_READWRITE | GDT_EXECUTABLE, GDT_FLAGS_32BIT);
    gdt->GDT[2] = gdt_entry_create(0, 0xffffffff, GDT_PRESENT | GDT_READWRITE                 , GDT_FLAGS_32BIT);

    // 64-bit code and data descriptors 0x18 0x20
    gdt->GDT[3] = gdt_entry_create(0, 0xffffffff, GDT_PRESENT | GDT_READWRITE | GDT_EXECUTABLE, GDT_FLAGS_64BIT_CODE);
    gdt->GDT[4] = gdt_entry_create(0, 0xffffffff, GDT_PRESENT | GDT_READWRITE                 , GDT_FLAGS_64BIT);
}

// *Install the GDT 
// @param gdt the gdt pointer to the given cpu's gdt entry
void install_gdt(struct cpu_GDT* gdt) {
    uint64_t gdt_address;
    struct GDT_pointer gdt_ptr;

    gdt_address = (uint64_t)gdt;
    gdt_ptr.size = sizeof(struct cpu_GDT);
    gdt_ptr.offset = gdt_address;

    ks.dbg("GDT built at %x. Loading to register...", gdt_address);
    load_gdt((uint32_t)&gdt_ptr);
}

void inline install_tss() {
   __asm__ volatile("ltr %%ax" : : "a"(0x28));
}

// === PUBLIC FUNCTIONS =========================

//* Load the Global Descriptor Table for the bootstrap processor
void init_gdt() {
    setup_gdt(&gdt_array[0]);
    install_gdt(&gdt_array[0]);
    
	ks.log("Global Descriptor Table loaded successfully.");
}

// *Load the Global Descriptor Table for the given processor
// @param cpu_id the processor id
void init_gdt_on_ap(uint32_t cpu_id) {
    setup_gdt(&gdt_array[cpu_id]);
    install_gdt(&gdt_array[cpu_id]);

    ks.log("Global Descriptor Table loaded successfully for CPU %u", cpu_id);
}

// *Setup the TSS
// @param cpu the structure information of the cpu
void init_tss(Cpu* cpu) {
    ks.dbg("Setting up TSS...");
    // tss descriptor 0x28
    struct TSS_entry entry = tss_entry_create(&(cpu->tss), (uint64_t)&(cpu->tss) + sizeof(cpu->tss), GDT_TSS_PRESENT | GDT_TSS, GDT_FLAGS_TSS);
    gdt_array[cpu->id].TSS = entry;

    // setup tss.ist1
    cpu->tss.iopb_offset = sizeof(Tss);
    cpu->tss.rsp0 = (uint64_t)cpu->stack;
    cpu->stack_interrupt = pmm_alloc_series(CPU_STACK_SIZE / PAGE_SIZE);
    cpu->tss.ist1 = CPU_STACK_BASE + CPU_STACK_SIZE;

    for (uint32_t i = 0; i < CPU_STACK_SIZE / PAGE_SIZE; i++)
        vmm_map_page(cpu->page_table, cpu->stack_interrupt + i*PAGE_SIZE, CPU_STACK_BASE + i*PAGE_SIZE, true, false);

    install_tss();
    ks.dbg("TSS set up for CPU #%u", cpu->id);
}
