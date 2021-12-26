#include "idt.h"
#include "kservice.h"
#include "kernel/common/device/port.h"
#include "libs/libc/stdint.h"

struct IDT_entry IDT[IDT_SIZE];

const char *interrupt_exception_name[] = {
    "Division By 0",
    "Debug interrupt",
    "NMI (Non Maskable Interrupt)",
    "Breakpoint interrupt",
    "invalid", 
    "table overflow",
    "Invalid opcode",
    "No FPU",
    "Double fault",
    "invalid", 
    "invalid TSS",
    "Segment not present",
    "invalid stack",
    "General protection fault",
    "Page fault",
    "invalid",
    "x87 FPU fault",
    "Alignment fault",
    "Machine check fault",
    "SIMD floating point exception",
    "vitualisation excpetion",
    "control protection exception",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid"
	};

// === PRIVATE FUNCTIONS ========================

// *Add a new entry to the IDT table
// @param irq the Interrupt number
// @param isr the pointer to the Interrupt Service Routine
// @param ist the offset of the interrupt stack table stored in the TSS
// @param type_attr the attributes for the newly added idt entry
void set_idt_entry(uint32_t irq, int(*isr)(), uint16_t ist, uint8_t type_attr) {
	uint64_t irq_address = (uint64_t)isr;
	IDT[irq].offset_lowerbits = (irq_address) & 0xffff;
	IDT[irq].selector = KERNEL_CODE;
	IDT[irq].zero = 0;
	IDT[irq].ist = ist;
	IDT[irq].type_attr = type_attr;
	IDT[irq].offset_middlebits = ((irq_address) >> 16) & 0xffff;
	IDT[irq].offset_higherbits = ((irq_address) >> 32) & 0xffffffff;
    IDT[irq].zero2 = 0;
}

// *Log the interrupt stack passed to the function. Useful for debugging interrupts stack frames
// @param stack the pointer to the interrupt stack to be logged
void log_interrupt_stack(interrupt_stack* stack) {
    ks.dbg(" ========== STACK FRAME LOG ==========");
    ks.dbg("rax: %x rbx: %x rcx: %x rdx: %x", stack->rax, stack->rbx, stack->rcx, stack->rdx);
    ks.dbg("rsi: %x rdi: %x rbp: %x", stack->rsi, stack->rdi, stack->rbp);
    ks.dbg("cs: %x ss: %x rip: %x", stack->cs, stack->ss, stack->rip);
    ks.dbg(" ====== END OF STACK FRAME LOG =======");
}

// === PUBLIC FUNCTIONS =========================

// *Initialize the Interrupt Descriptor Table
void init_idt() {
	for (uint64_t i = 0; i < 48; i++) {
		if (i == 14 || i == 32) 
			set_idt_entry(i, _interrupt_vector[i], 1, INTERRUPT_GATE);
		else
			set_idt_entry(i, _interrupt_vector[i], 0, INTERRUPT_GATE);
	}

    /* fill the IDT descriptor */
	struct IDT_pointer idt_ptr; 
	idt_ptr.size = sizeof(struct IDT_entry) * IDT_SIZE;
	idt_ptr.offset = (uint64_t)&IDT[0];

    ks.dbg("IDT built at %x, size is %x. Loading to register...", idt_ptr.offset, idt_ptr.size);
	load_idt((uint32_t)&idt_ptr);
	ks.log("Interrupt Descriptor Table loaded successfully.");
}

interrupt_stack* exception_handler(interrupt_stack* stack) {
	ks.warn("got exception: %c (%u), error code: %x, cs: %x", interrupt_exception_name[stack->irq], stack->irq, stack->error_code, stack->cs);
    log_interrupt_stack(stack);

    apic_eoi();
    return stack;
}

interrupt_stack* interrupt_handler(interrupt_stack* stack) {
	ks.log("got interrupt %u", stack->irq);
    log_interrupt_stack(stack);

    apic_eoi();
    return stack;
}

void pagefault_handler(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n) {
	unsigned long faulting_address, temp;
   	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

	int present = !(m & 0x1);
	int rw = m & 0x2;           // Write operation?
    int us = m & 0x4;           // Processor was in user-mode?
    int reserved = m & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = m & 0x10; 

	ks.err("Page fault at address %x", faulting_address); 
	asm("hlt");
}