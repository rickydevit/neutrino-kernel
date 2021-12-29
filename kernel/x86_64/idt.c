#include "idt.h"
#include "kservice.h"
#include "pic.h"
#include "device/time/pit.h"
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
void log_interrupt(interrupt_stack* stack) {
    ks.dbg(" ========== INTERRUPT FRAME LOG ==========");
    ks.dbg("interrupt: %u error code: %x", stack->irq, stack->error_code);
    ks.dbg("rax: %x rbx: %x rcx: %x rdx: %x", stack->rax, stack->rbx, stack->rcx, stack->rdx);
    ks.dbg("rsi: %x rdi: %x rbp: %x", stack->rsi, stack->rdi, stack->rbp);
    ks.dbg("r8: %x r9: %x r10: %x", stack->r8, stack->r9, stack->r10);
    ks.dbg("r11: %x r12: %x r13: %x", stack->r11, stack->r12, stack->r13);
    ks.dbg("r14: %x r15: %x", stack->r14, stack->r15);
    ks.dbg("rflags: %x rsp: %x", stack->rflags, stack->rsp);
    ks.dbg("cs: %x ss: %x rip: %x", stack->cs, stack->ss, stack->rip);
    ks.dbg("pic isr: %x  pic irr: %x", pic_get_isr(), pic_get_irr());
    ks.dbg(" ====== END OF INTERRUPT FRAME LOG =======");
}

// === PUBLIC FUNCTIONS =========================

// *Initialize the Interrupt Descriptor Table
void init_idt() {
	for (uint64_t i = 0; i < 48; i++) {
		if (i == 14 || i == 32) 
			set_idt_entry(i, _interrupt_vector[i], 0, INTERRUPT_GATE);
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

void disable_interrupts() {
    asm volatile ("cli");
}

void enable_interrupts() {
    asm volatile ("sti");
}

interrupt_stack* exception_handler(interrupt_stack* stack) {
	ks.warn("got exception: %c (%u)", interrupt_exception_name[stack->irq], stack->irq);
    log_interrupt(stack);

    switch (stack->irq) {
        case 8:     // DF
        case 10:    // TS
        case 11:    // NP
        case 12:    // SS
        case 13:    // GP
        case 17:    // AC
        case 18:    // MC
        case 21:    // CP
        case 29:    // VC
        case 30:    // SX
            ks.err("Neutrino encountered a fatal exception! Cannot proceed.", stack->rip);
            break;
        case 14:    // PF
            pagefault_handler(stack);
    }

    apic_eoi();
    return stack;
}

interrupt_stack* interrupt_handler(interrupt_stack* stack) {
    ks.dbg("Got interrupt %u", stack->irq);

    apic_eoi();
    return stack;
}

void pagefault_handler(interrupt_stack* stack) {
	uint64_t faulting_address;
   	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

	int present = !(stack->error_code & 0x1);
	int rw = stack->error_code & 0x2;           // Write operation?
    int us = stack->error_code& 0x4;           // Processor was in user-mode?
    int reserved = stack->error_code& 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = stack->error_code& 0x10; 

	ks.err("Page fault at address %x", faulting_address); 
	asm("hlt");
}